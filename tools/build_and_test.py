"""[수정] MSBuild FileTracker 없이 설치된 MSVC로 원본 소스와 회귀 테스트를 빌드한다."""
import os
from pathlib import Path
import subprocess
import sys
import shutil

ROOT = Path(__file__).resolve().parents[1]
VS = Path(os.environ.get('ProgramFiles', r'C:\Program Files')) / 'Microsoft Visual Studio/18/Community'
if not VS.exists():
    raise SystemExit('Visual Studio 18 Community not found; run this script from a VS developer prompt or update VS path.')
env = {key.upper(): value for key, value in os.environ.items()}
setup = f'""{VS / "Common7/Tools/VsDevCmd.bat"}" -arch=x64 -host_arch=x64 >nul && set"'
result = subprocess.run('cmd.exe /d /s /c ' + setup, env=env, capture_output=True, text=True)
if result.returncode:
    raise SystemExit(result.stdout + result.stderr)
for line in result.stdout.splitlines():
    if '=' in line and not line.startswith('='):
        key, value = line.split('=', 1)
        env[key.upper()] = value
out = ROOT / 'build/validation'
out.mkdir(parents=True, exist_ok=True)
src = ROOT / 'BhaviorTreeTest_Project'
common = [p for p in src.glob('*.cpp') if p.name not in ('WinThread.cpp', 'BhaviorTreeTest_Project.cpp')]
flags = ['/nologo', '/std:c++17', '/EHsc', '/utf-8', '/W3', '/DUNICODE', '/D_UNICODE', '/DNOMINMAX', '/D__FIX_SEED__', '/MDd', '/Zi', '/Od', '/RTC1', f'/I{src}']
if '--asan' in sys.argv:
    flags.remove('/RTC1')
    flags.append('/fsanitize=address')

def run(args, cwd=out):
    args[0] = shutil.which(str(args[0]), path=env['PATH']) or args[0]
    subprocess.run([str(x) for x in args], cwd=cwd, env=env, check=True)

run(['cl', *flags, '/c', *common, f'/Fo{out}\\', f'/Fd{out / "core.pdb"}'])
objects = [out / (p.stem + '.obj') for p in common]
libraries = ['user32.lib', 'gdi32.lib', 'gdiplus.lib', 'winmm.lib']
run(['cl', *flags, src / 'WinThread.cpp', src / 'BhaviorTreeTest_Project.cpp', *objects, f'/Fe{out / "simulator.exe"}', '/link', *libraries])
run(['cl', *flags, ROOT / 'tests/SimulationTests.cpp', *objects, f'/Fe{out / "simulation_tests.exe"}', '/link', *libraries])
env['PATH'] = str(VS / 'VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64') + os.pathsep + env['PATH']
run([out / 'simulation_tests.exe'], cwd=src)
if '--benchmark' in sys.argv:
    bench_flags = [flag for flag in flags if flag not in ('/MDd', '/Od', '/RTC1', '/fsanitize=address')]
    bench_flags += ['/MD', '/O2', '/DNDEBUG']
    bench_out = out / 'benchmark'
    bench_out.mkdir(exist_ok=True)
    run(['cl', *bench_flags, ROOT / 'tests/ReservationBenchmark.cpp', src / 'CTeam.cpp', f'/Fe{out / "reservation_benchmark.exe"}'], cwd=bench_out)
    result = subprocess.run([str(out / 'reservation_benchmark.exe')], cwd=src, env=env, text=True, capture_output=True, check=True)
    (ROOT / 'BenchmarkResults.txt').write_text(result.stdout, encoding='utf-8')
    print(result.stdout)

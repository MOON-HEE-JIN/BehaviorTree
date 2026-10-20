"""[수정] 독립 프로세스 반복 실행과 수정 금지 파일 무변경을 기록한다."""
from pathlib import Path
import hashlib
import os
import subprocess
import time

root = Path(__file__).resolve().parents[1]
exe = root / 'build/validation/simulation_tests.exe'
env = {key.upper(): value for key, value in os.environ.items()}
runtime = Path(r'C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64')
env['PATH'] = str(runtime) + os.pathsep + env['PATH']
outputs = []
lines = ['Validation date: 2026-10-20 (Asia/Seoul)',
         'Compiler: MSVC 14.51.36231 x64; C++17; /MDd /Zi /Od /fsanitize=address',
         'Seed: __FIX_SEED__ => mt19937(19937); test executable in independent processes',
         'Native GUI executable: compiled/linked; interactive window lifecycle not exercised',
         'MSBuild: blocked by FileTracker E_ACCESSDENIED; direct installed MSVC used instead']
for number in range(2):
    start = time.perf_counter()
    result = subprocess.run([str(exe)], cwd=root / 'BhaviorTreeTest_Project', env=env, capture_output=True, timeout=60)
    duration = time.perf_counter() - start
    assert result.returncode == 0, (result.returncode, result.stderr)
    assert b'AddressSanitizer' not in result.stderr, result.stderr
    outputs.append(result.stdout)
    lines.append(f'Run {number + 1}: exit=0, elapsed={duration:.3f}s, stdout_sha256={hashlib.sha256(result.stdout).hexdigest()}')
    lines.extend(line for line in result.stdout.decode('utf-8', errors='replace').splitlines()
                 if line.startswith(('PASS:', 'FULL SIMULATION:')))
assert outputs[0] == outputs[1], 'Independent fixed-seed executions differ'
lines.append('Independent executions: identical full stdout; ASan reports: none on exercised inputs')
for name in ('CAstar.h', 'CAstar.cpp', 'CTimerWheel.h', 'CTimerWheel.cpp', 'CTimerWheelManager.h', 'CTimerWheelManager.cpp'):
    path = 'BhaviorTreeTest_Project/' + name
    original = subprocess.check_output(['git', 'show', 'HEAD:' + path], cwd=root)
    current = (root / path).read_bytes()
    # git text checkout may use CRLF; compare canonical LF bytes without rewriting the protected files.
    assert original.replace(b'\r\n', b'\n') == current.replace(b'\r\n', b'\n'), name
    lines.append(f'Protected file unchanged: {path}')
lines.append('Not exercised: Astar isolated-start crash, optimality oracle, full timer boundary matrix, CRT leaks, UBSan/TSan, GUI stress, whole Tick p95/p99/allocation counters.')
(root / 'TestResults.txt').write_text('\n'.join(lines) + '\n', encoding='utf-8')
print('\n'.join(lines))

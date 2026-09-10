#!/usr/bin/env python3
from pathlib import Path
root = Path(__file__).resolve().parents[1]

def rels(base):
    return sorted(p.relative_to(root).as_posix() for p in base.rglob('*.cpp'))

runtime = [p for p in rels(root / 'src') if p != 'src/main.cpp']
all_tests = rels(root / 'tests')
test_mains, test_support = [], []
for rp in all_tests:
    text = (root / rp).read_text(errors='ignore')
    (test_mains if ('int main(' in text or 'int main (' in text) else test_support).append(rp)
tools = rels(root / 'tools')

lines = [
    '# Generated deterministic production source inventory. Do not hand-edit.',
    '# Regenerate with tools/generate_production_sources.py.',
    f'set(ELYSIUM_EXPECTED_RUNTIME_SOURCE_COUNT {len(runtime)})',
    f'set(ELYSIUM_EXPECTED_TEST_SOURCE_COUNT {len(all_tests)})',
    f'set(ELYSIUM_EXPECTED_TOOL_SOURCE_COUNT {len(tools)})', '',
]
for var, items in (
    ('ELYSIUM_PRODUCTION_RUNTIME_SOURCES', runtime),
    ('ELYSIUM_PRODUCTION_TEST_MAIN_SOURCES', test_mains),
    ('ELYSIUM_PRODUCTION_TEST_SUPPORT_SOURCES', test_support),
    ('ELYSIUM_PRODUCTION_TOOL_SOURCES', tools),
):
    lines.append(f'set({var}')
    lines.extend(f'    {p}' for p in items)
    lines.extend([')', ''])
(root / 'cmake' / 'ProductionSources.cmake').write_text('\n'.join(lines))
print(f'Wrote production inventory: {len(runtime)} runtime, {len(all_tests)} tests, {len(tools)} tools.')

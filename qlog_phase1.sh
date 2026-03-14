#!/usr/bin/env bash
# QLog v0.49.0 merge — PHASE 1
# Run from: ~/QLog on wspr.hb9vqq.ch
# Exits on first error. Review output at each CHECKPOINT comment.
set -euo pipefail

echo "=== A1: Confirm upstream remote ==="
if ! git remote | grep -q upstream; then
  echo "Adding upstream remote..."
  git remote add upstream https://github.com/foldynl/QLog.git
fi
git remote -v | grep upstream
# Expected: upstream  https://github.com/foldynl/QLog.git (fetch/push)

echo ""
echo "=== A2: Save Band Offsets UI block ==="
grep -n 'antBandOffsetsLabel\|antBandOffsetsButton' ui/SettingsDialog.ui
# Note line numbers printed above; block should be ~573-600
sed -n '573,600p' ui/SettingsDialog.ui > /tmp/bandoffsets_block.txt
cat /tmp/bandoffsets_block.txt
echo "CHECKPOINT A2: verify both row-4 antBandOffsetsLabel and antBandOffsetsButton items are in /tmp/bandoffsets_block.txt"
echo "Press Enter to continue or Ctrl-C to abort..."
read -r

echo ""
echo "=== B1: Fetch upstream and start merge ==="
git fetch upstream
# merge will likely stop with conflicts — that is expected
git merge upstream/master || true

echo ""
echo "=== B2: Verify conflict list ==="
CONFLICTS=$(git diff --name-only --diff-filter=U)
echo "Conflicted files:"
echo "$CONFLICTS"
echo ""
echo "CHECKPOINT B2: Compare against expected 23-file list in the plan."
echo "Any unexpected file needs manual assessment before proceeding."
echo "Press Enter to continue or Ctrl-C to abort..."
read -r

echo ""
echo "=== C1: Standard keep-HEAD files ==="
git checkout --ours \
  README.md \
  core/AlertEvaluator.cpp \
  core/LogParam.cpp \
  core/Migration.h \
  core/main.cpp \
  data/BandPlan.cpp \
  data/BandPlan.h \
  installer/config/config.xml \
  "installer/packages/de.dl2ic.qlog/meta/package.xml" \
  res/res.qrc \
  res/sql/migration_036.sql \
  ui/DxFilterDialog.cpp \
  ui/DxFilterDialog.ui \
  ui/DxWidget.cpp
echo "keep-HEAD files checked out."

echo ""
echo "=== C1b: Revert Migration.cpp to HEAD (silent auto-merge fix) ==="
git checkout HEAD -- core/Migration.cpp
grep 'class Migration\|class DBSchemaMigration' core/Migration.cpp
# MUST show: Migration (not DBSchemaMigration)

echo ""
echo "=== C1c: Update installer version strings ==="
sed -i 's/<Version>0\.48\.0<\/Version>/<Version>0.49.0<\/Version>/' installer/config/config.xml
grep 'Version' installer/config/config.xml
# Must show 0.49.0

sed -i 's/<Version>0\.48\.0-1<\/Version>/<Version>0.49.0-1<\/Version>/' "installer/packages/de.dl2ic.qlog/meta/package.xml"
sed -i 's/<ReleaseDate>2026-01-30<\/ReleaseDate>/<ReleaseDate>2026-03-13<\/ReleaseDate>/' "installer/packages/de.dl2ic.qlog/meta/package.xml"
grep 'Version\|ReleaseDate' "installer/packages/de.dl2ic.qlog/meta/package.xml"
# Must show 0.49.0-1 and 2026-03-13

echo ""
echo "=== C2: QLog.pro — manual merge required ==="
echo "MANUAL STEP: Edit QLog.pro now. Remove ALL conflict markers (<<<, ===, >>>)."
echo "Keep ALL .cpp/.h lines from BOTH HEAD and upstream. Save the file."
echo "Press Enter when done..."
read -r
# Verify no duplicates
DUPES=$(sort QLog.pro | uniq -d | grep -E '\.(cpp|h)' || true)
if [ -n "$DUPES" ]; then
  echo "ERROR: Duplicate lines found in QLog.pro:"
  echo "$DUPES"
  echo "Fix duplicates before continuing. Press Enter when ready..."
  read -r
else
  echo "OK: No duplicate .cpp/.h lines in QLog.pro."
fi

echo ""
echo "=== C2b: Update QLog.pro VERSION ==="
sed -i 's/^VERSION = 0\.48\.0/VERSION = 0.49.0/' QLog.pro
grep '^VERSION' QLog.pro
# Must show: VERSION = 0.49.0

echo ""
echo "=== C3: MainWindow.cpp — verify includes ==="
# This file may have auto-merged or conflicted; resolve if needed
if git diff --name-only --diff-filter=U | grep -q 'ui/MainWindow.cpp'; then
  echo "MANUAL STEP: MainWindow.cpp still conflicted. Resolve now — keep both include sets."
  echo "Our addition: #include \"core/PropagationData.h\" (~line 24)"
  echo "Upstream adds 12 includes (~line 36). Ensure ALL are present."
  echo "Press Enter when done..."
  read -r
fi
grep '#include "core/PropagationData.h"' ui/MainWindow.cpp
grep '#include "core/LogDatabase.h"' ui/MainWindow.cpp
grep 'updateCurrentBand' ui/MainWindow.cpp
echo "OK: MainWindow.cpp includes verified."

echo ""
echo "=== C4: SettingsDialog.cpp — verify includes ==="
if git diff --name-only --diff-filter=U | grep -q 'ui/SettingsDialog.cpp'; then
  echo "MANUAL STEP: SettingsDialog.cpp still conflicted."
  echo "Keep: BandOffsetDialog.h (ours), RigctldAdvancedDialog.h + CWWinKey.h (upstream)."
  echo "Constructor region: keep upstream additions (m_tqslVersionTimer, countyCompleter)."
  echo "Press Enter when done..."
  read -r
fi
grep 'BandOffsetDialog\|RigctldAdvancedDialog\|CWWinKey' ui/SettingsDialog.cpp

echo ""
echo "=== C5: SettingsDialog.h — verify ==="
if git diff --name-only --diff-filter=U | grep -q 'ui/SettingsDialog.h'; then
  echo "MANUAL STEP: SettingsDialog.h still conflicted. Merge both sides."
  echo "Ours:     editBandOffsets() slot, QMap<QString,double> tempBandOffsets member"
  echo "Upstream: QTimer include, tqslAutoDetect, updateTQSLVersionLabel,"
  echo "          showRigctldAdvanced, rigShareChanged slots, countyCompleter, m_tqslVersionTimer"
  echo "Press Enter when done..."
  read -r
fi
grep 'editBandOffsets\|tempBandOffsets\|tqslAutoDetect\|showRigctldAdvanced' ui/SettingsDialog.h

echo ""
echo "=== C6: SettingsDialog.ui — take upstream ==="
git checkout --theirs ui/SettingsDialog.ui
echo "Upstream SettingsDialog.ui checked out."

echo ""
echo "=== C7: Re-insert Band Offsets block into SettingsDialog.ui ==="
grep -n 'antAzOffsetSpinBox' ui/SettingsDialog.ui
echo ""
echo "MANUAL STEP: Insert /tmp/bandoffsets_block.txt immediately after the"
echo "</item> that closes the antAzOffsetSpinBox row (row 3 in antenna grid)."
echo "Use your editor. Save the file."
echo "Press Enter when done..."
read -r
COUNT=$(grep -c 'antBandOffsetsLabel\|antBandOffsetsButton' ui/SettingsDialog.ui || true)
if [ "$COUNT" -ne 2 ]; then
  echo "ERROR: Expected 2 occurrences of antBandOffsetsLabel/antBandOffsetsButton, got $COUNT"
  echo "Fix before continuing. Press Enter when ready..."
  read -r
else
  echo "OK: Band Offsets block verified (count=$COUNT)."
fi

echo ""
echo "=== C7b: Check for any remaining conflicts ==="
REMAINING=$(git diff --name-only --diff-filter=U 2>/dev/null || true)
if [ -n "$REMAINING" ]; then
  echo "Still conflicted:"
  echo "$REMAINING"
  echo ""
  echo "Fallback resolutions per plan:"
  echo "  core/LogParam.h       -> git checkout --ours, re-add upstream getBandmapShowEmergency/getUploadLoTWLocation"
  echo "  rotator/HamlibRotDrv  -> git checkout --ours, apply upstream's 359.9 -> 359.9f fix"
  echo "  ui/MainWindow.ui      -> git checkout --theirs, verify propagationDockWidget present"
  echo "  ui/NewContactWidget   -> git checkout --theirs, verify updateCurrentBand Rotator call"
  echo "Resolve all above, then press Enter..."
  read -r
fi

echo ""
echo "=== C8: Stage and verify ==="
git add -A
git status
STILL_CONFLICTED=$(git status --porcelain | grep '^UU\|^AA\|^DD' || true)
if [ -n "$STILL_CONFLICTED" ]; then
  echo "ERROR: Files still in conflict state:"
  echo "$STILL_CONFLICTED"
  exit 1
fi
echo "OK: No files remaining in 'both modified' state."

echo ""
echo "=== D1: Commit and push ==="
echo "About to: git commit (merge commit) + git push"
echo "Press Enter to proceed or Ctrl-C to abort..."
read -r
git commit --no-edit
echo ""
echo "Enter your GitHub token for push (will not echo):"
read -rs GH_TOKEN
git push "https://HB9VQQ:${GH_TOKEN}@github.com/HB9VQQ/QLog.git" master
echo ""
echo "=== PHASE 1 COMPLETE ==="
echo "Expected CI: FAIL on getPasswd / dbFilename / isFTxMode / CredentialStore."
echo "Check: https://github.com/HB9VQQ/QLog/actions"
echo "Verify CI errors match known list before starting Phase 2."

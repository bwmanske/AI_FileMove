#!/bin/bash

# Exit on error
set -e

# Use absolute path for the project root to avoid ambiguity
PROJECT_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SANDBOX_DIR="$PROJECT_ROOT/gemma4-win32/tests/e2e_sandbox"
BINARY_PATH="$PROJECT_ROOT/gemma4-win32/build/FileMove"

# --- SETUP ---
echo "--- Setting up sandbox ---"
rm -rf "$SANDBOX_DIR"
mkdir -p "$SANDBOX_DIR/src/dir1/subdir1"
mkdir -p "$SANDBOX_DIR/dest/groupA/dest1"
touch "$SANDBOX_DIR/src/file1.txt"
touch "$SANDBOX_DIR/src/dir1/subdir1/file2.txt"

# Create config inside sandbox
cat <<EOF > "$SANDBOX_DIR/config.json"
{
  "version": 1,
  "lastSelectedGroupId": "",
  "sortMode": 0,
  "placementMode": 0,
  "windowWidth": 400,
  "windowHeight": 500,
  "windowLeft": 100,
  "windowTop": 100,
  "enableDirectoryMoves": true,
  "enableSidecarFiles": true,
  "hideQueuedSourceFiles": false,
  "groups": [
    {
      "id": "group-123",
      "name": "groupA",
      "destinationPaths": ["dest/groupA/dest1"],
      "defaultAction": "",
      "createdAt": "2024-01-01 00:00:00",
      "updatedAt": "2024-01-01 00:00:00",
      "lastUsedAt": "2024-01-01 00:00:00"
    }
  ]
}
EOF

echo "--- Starting E2E Test Scenario: Directory Expansion ---"

# 2. Run FileMove via CLI
cd "$SANDBOX_DIR"
echo "Running FileMove with config: config.json"
$BINARY_PATH -I config.json -D MV -S AZ /O output.log src/file1.txt src/dir1

# 3. Verification
echo "Verifying results..."

# Check if file1.txt was moved
if [ -f "dest/groupA/dest1/file1.txt" ]; then
    echo "[PASS] file1.txt successfully moved."
else
    echo "[FAIL] file1.txt not found in destination!"
    exit 1
fi

# Check if directory expansion worked (dir1/subdir1/file2.txt)
if [ -f "dest/groupA/dest1/subdir1/file2.txt" ]; then
    echo "[PASS] Directory expansion successful for file2.txt."
else
    echo "[FAIL] Directory expansion failed!"
    find . -name "file2.txt"
    exit 1
fi

# Check if logs were created
if [ -f "output.log" ]; then
    echo "[PASS] Log file created."
else
    echo "[FAIL] Log file not found!"
    exit 1
fi

echo "--- E2E Test Completed Successfully! ---"

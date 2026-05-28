# FileMove User Guide

FileMove is a compact Windows utility designed to facilitate rapid file organization by allowing users to send files from Windows Explorer directly into pre-defined destination groups.

## Key Features
- **Destination Groups**: Save one or more folder paths under a single name (e.g., "Invoices 2026").
- **Explorer Integration**: Drag and drop files from Explorer onto a group in the app, or use the context menu/clipboard.
- **Compact UI**: A dense interface that stays out of your way while remaining easily accessible.

## Command Line Interface
FileMove supports several command-line arguments for automation and customization. All options must be preceded by `/` or `-`.

| Option | Value | Description |
| :--- | :--- | :--- |
| `/D` | `MV` or `CP` | **Debug Mode**: `MV` is normal move behavior. `CP` is debug copy mode (does not remove source). |
| `/I` | `<path>` | **Input JSON**: Specifies an alternate JSON file for groups and settings. |
| `/O` | `<path>` | **Output Log**: Specifies an alternate log file path. |
| `/S` | `<value>` | **Sort Order**: Sets startup sort (`MRU`, `LRU`, `AF`, `AL`, `AZ`, `ZA`). |
| `/P` | `<value>` | **Placement**: Sets window position (`UL`, `UR`, `LL`, `LR`, `LAST`). |

**Example:**
`FileMove.exe -D MV -I C:\Data\Groups.json /S MRU`

## Usage Workflow
1. **Create a Group**: Click `New / Settings` and enter a name and one or more destination folders.
2. **Send Files**: 
   - Drag files from Windows Explorer and drop them onto a group row in the app.
   - Copy files in Explorer, right-click a group in FileMove, and select `Use Clipboard`.
3. **Monitor Status**: Use the `Status` window to view active transfers, queue status, and recent log entries.

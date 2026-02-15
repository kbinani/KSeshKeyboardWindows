# KSesh IME

A Windows IME (Input Method Editor) for Egyptological transliteration, enabling easy input of special characters used in Egyptology.

## Features

- Automatic conversion of standard characters to Egyptological transliteration characters
- Customizable character replacement options

## Character Mappings

| Input | Output | Description |
|-------|--------|-------------|
| D | ḏ | U+1E0F - Latin Small Letter D with Line Below |
| T | Ṯ | U+1E6E - Latin Capital Letter T with Line Below |
| A | ꜣ or Ꜣ | U+A723 or U+A722 - Egyptological Alef (lowercase/uppercase, customizable) |
| H | ḥ | U+1E25 - Latin Small Letter H with Dot Below |
| x | ḫ | U+1E2B - Latin Small Letter H with Breve Below |
| X | ẖ | U+1E96 - Latin Small Letter H with Line Below |
| S | š | U+0161 - Latin Small Letter S with Caron |
| a | ꜥ or Ꜥ | U+A725 or U+A724 - Egyptological Ain (lowercase/uppercase, customizable) |
| i | ꞽ (default) | Multiple options available (customizable, see below) |
| q | q or ḳ | Unchanged or U+1E33 (K with Dot Below) - customizable |
| Y | ï or Y | U+00EF (I with Diaeresis) or unchanged - customizable, default: ï |

## Configurable Options

### Letter "i" Replacement

You can choose from multiple options for replacing the letter "i":

- **ı͗** (ı + U+0357) - Dotless I with Combining Right Half Ring Above
- **i͗** (i + U+0357) - I with Combining Right Half Ring Above
- **i҆** (i + U+0486) - I with Combining Cyrillic Psili Pneumata
- **i̯** (i + U+032F) - I with Combining Inverted Breve Below
- **ꞽ** (U+A7BD) - Latin Small Letter Glottal I (default)
- **i** - Unchanged

### Letter "q" Replacement

- **Off** (default): "q" remains unchanged
- **On**: "q" → **ḳ** (U+1E33, Latin Small Letter K with Dot Below)

### Letter "Y" Replacement

- **Off**: "Y" remains unchanged
- **On** (default): "Y" → **ï** (U+00EF, Latin Small Letter I with Diaeresis)

### Capital Aleph

- **Off** (default): "A" → **ꜣ** (lowercase aleph)
- **On**: "A" → **Ꜣ** (U+A722, uppercase aleph)

### Capital Ain

- **Off** (default): "a" → **ꜥ** (lowercase ain)
- **On**: "a" → **Ꜥ** (U+A724, uppercase ain)

## Building from Source

### Requirements

- Visual Studio 2022 or later
- Windows SDK

### Build Steps

Open `KSeshKeyboardWindows.slnx` and build it. The compiled DLL will be located in `x64\Release\KSeshIME.dll`.

### Creating the Installer

The project includes an Inno Setup script at `pkg\installer.iss` to create an installer.

## License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details.

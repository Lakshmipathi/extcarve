# Extcarve Changelog

## Version 1.5 (18-Nov-2025)

### Major Feature Addition: Comprehensive File Format Support

Added support for **33 new file formats**, bringing total supported formats from 17 to **50 file formats**.

#### New Image Formats (6):
- **BMP** - Windows Bitmap (.bmp)
- **TIFF** - Tagged Image File Format (.tiff) - Both Little & Big Endian
- **WebP** - Google WebP (.webp)
- **ICO** - Windows Icon (.ico)
- **PSD** - Adobe Photoshop Document (.psd)

#### New Audio Formats (5):
- **WAV** - Waveform Audio File Format (.wav)
- **FLAC** - Free Lossless Audio Codec (.flac)
- **OGG** - Ogg Vorbis (.ogg)
- **M4A/AAC** - MPEG-4 Audio (.m4a)
- **WMA** - Windows Media Audio (.wma)

#### New Video Formats (6):
- **MP4** - MPEG-4 Video (.mp4)
- **AVI** - Audio Video Interleave (.avi)
- **MKV** - Matroska Video (.mkv)
- **MOV** - QuickTime Movie (.mov)
- **FLV** - Flash Video (.flv)
- **MPEG** - Moving Picture Experts Group (.mpeg)

#### New Document Formats (5):
- **DOCX** - Microsoft Word (Office Open XML) (.docx)
- **XLSX** - Microsoft Excel (Office Open XML) (.xlsx)
- **PPTX** - Microsoft PowerPoint (Office Open XML) (.pptx)
- **RTF** - Rich Text Format (.rtf)
- **ODT** - OpenDocument Text (.odt)

#### New Archive Formats (3):
- **RAR** - Roshal Archive (.rar)
- **7Z** - 7-Zip Archive (.7z)
- **XZ** - XZ Compressed Archive (.xz)

#### New Executable Formats (1):
- **EXE/PE** - Windows Portable Executable (.exe)

#### New Database & Other Formats (3):
- **SQLite** - SQLite Database (.db)
- **PST** - Outlook Personal Storage (.pst)
- **ISO** - ISO 9660 CD/DVD Image (.iso)

### Technical Improvements:
- Increased `dotpart` buffer size from 4 to 6 bytes to support longer extensions (.docx, .xlsx, .pptx, .tiff, .mpeg, .webp)
- Increased `FILE_TYPE` buffer size from 5 to 6 bytes
- Enhanced user prompts with organized, categorized file type listings
- Improved footer detection for ZIP-based formats (DOCX, XLSX, PPTX, ODT)
- Added EOF detection support for multimedia and archive formats

### File Format Magic Bytes Reference:
All file formats are detected using industry-standard magic byte signatures from file format specifications.

---

## Version 1.4 (11-Aug-2013)
- MP3 format added

## Version 1.3 (Nov-2011)
- ELF format support added

## Version 1.2 (Oct-2011)
- RPM format support added

## Version 1.1 (Jul-2011)
- Analyze mode (-a) implementation

## Version 1.0 (Jul-2011)
- BZIP2 format support
- ASCII text file support

## Version 0.7 (Jul-2011)
- GZIP/TGZ and ZIP format support

## Version 0.6
- Bug fixes and stability improvements

## Version 0.5 (Jun-2011)
- MATLAB .fig format support

## Version 0.4 (Jun-2011)
- LaTeX .tex format support

## Version 0.3 (Early development)
- Initial release with GIF, PNG, JPG, PDF, C/C++, PHP support

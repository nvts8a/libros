# libros

[Download the latest version of libros here.](https://github.com/nvts8a/libros/raw/main/releases/libros-LATEST.uf2)

Minor versions (0.0.1 > 0.0.2) represent no functional changes or features, sometimes bug fixes

Major versions (0.0.1 > 0.1.0) represent functional changes, features, or bug fixes that may break current files

- [0.2.3](https://github.com/nvts8a/libros/raw/main/releases/libros-0.2.2.uf2)
  - Fixed bug where the progress bar label starts all the way to the left regardless of the read percentage
  - Moving some view positions to consts in BookReaderViewController.h
- [0.2.2](https://github.com/nvts8a/libros/raw/main/releases/libros-0.2.2.uf2)
  - Fixed bug where the progress bar started at empty and only sized up to one less than completely full
  - Organized and commented BookReaderViewController::_updateView
- [0.2.1](https://github.com/nvts8a/libros/raw/main/releases/libros-0.2.1.uf2)
  - Fixed bug where you could turn the page past max book page size
  - BookReaderViewController refactors and code quaility improvements and comments
  - Moving some Focus.h defines to consts and shortening names
- [0.2.0](https://github.com/nvts8a/libros/raw/main/releases/libros-0.2.0.uf2)
  - A new folder is created when it doesn't exist, _DATABASE
  - _LIBRARY is now stored and read from _DATABASE
  - A new folder is created when it doesn't exist, _DATABASE/_PAGES
  - .pag and .opg files for books are now stored in _DATABASE/_PAGES
  - Continued OpenBookDatabase refactors and code quaility improvements and comments
  - OpenBookDevice now has a makeDirectory method
- [0.1.2](https://github.com/nvts8a/libros/raw/main/releases/libros-0.1.2.uf2)
  - Fixed bug where new books added wouldn't show up unless you restarted device
  - Organized and commented OpenBookDatabase::connect
  - Added more consts and removed unused numFields variable
- [0.1.1](https://github.com/nvts8a/libros/raw/main/releases/libros-0.1.1.uf2)
  - Comments in OpenBookDatabase.cpp
  - Moving some OpenBookDatabase.h defines to consts and shortening names
  - Expanded some variable names
  - Couple new consts for Pagination chars and Header tags
  - Added test books to resources directory
  - Added scrips directory with a couple things I'd like to move to PlatformIO

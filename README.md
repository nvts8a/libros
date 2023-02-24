# libros

- 0.2.0
  - A new folder is created when it doesn't exist, _DATABASE
  - _LIBRARY is now stored and read from _DATABASE
  - A new folder is created when it doesn't exist, _DATABASE/_PAGES
  - .pag and .opg files for books are now stored in _DATABASE/_PAGES
  - Continued OpenBookDatabase refactors and code quaility improvements and comments
  - OpenBookDevice now has a makeDirectory method
- 0.1.2
  - Fixed bug where new books added wouldn't show up unless you restarted device
  - Organized and commented OpenBookDatabase::connect
  - Added more consts and removed unused numFields variable
- 0.1.1
  - Comments in OpenBookDatabase.cpp
  - Moving some OpenBookDatabase.h defines to consts and shortening names
  - Expanded some variable names
  - Couple new consts for Pagination chars and Header tags
  - Added test books to resources directory
  - Added scrips directory with a couple things I'd like to move to PlatformIO

Minor versions (0.0.1 > 0.0.2) represent no functional changes or features, sometimes bug fixes

Major versions (0.0.1 > 0.1.0) represent functional changes, features, or bug fixes that may break current files
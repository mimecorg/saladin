include( config.pri )

TEMPLATE = subdirs
SUBDIRS  = src

translations.files = translations/*.qm translations/locale.ini
translations.path = $$PREFIX/translations
INSTALLS += translations

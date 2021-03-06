SCRIPTDIR=$(dirname "$0")
find "${SCRIPTDIR}/.." -path ./tests/googletest -prune -o -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format -i \
-style="{BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 160, PointerAlignment: Left, DerivePointerAlignment: false, AllowShortFunctionsOnASingleLine: Empty}"

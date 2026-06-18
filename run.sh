export BANKOMP_KEY="IME_SE9_SECRET_KEY_2026"

mkdir -p build
cd build

cmake ..
make

if [ $? -eq 0 ]; then
    clear
    ./bankomp
fi
cd ..
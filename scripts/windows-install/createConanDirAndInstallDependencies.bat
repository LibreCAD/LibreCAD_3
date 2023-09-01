echo "conan"
mkdir conan
pushd conan
conan profile detect --force
conan install .. -s build_type=Release --build missing
popd

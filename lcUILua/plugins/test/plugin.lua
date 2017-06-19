print(LC_interface)
file = openFile("/tmp/a", "r+")
actual = file:read(1)
actual = actual .. "Test"
print(actual)
file:write(actual)
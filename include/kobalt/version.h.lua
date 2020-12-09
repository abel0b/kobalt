function os.capture(cmd)
    local f = assert(io.popen(cmd, 'r'))
    local s = assert(f:read('*a'))
    f:close()
    s = string.gsub(s, '^%s+', '')
    s = string.gsub(s, '%s+$', '')
    s = string.gsub(s, '[\n\r]+', ' ')
    return s
end

if arg[1] then
    version = arg[1]
else
    version = os.capture("git describe --tags --abbrev=0 HEAD")
end
    
print("#ifndef KLVERSION__H")
print("#define KLVERSION__H")
print(string.format("#define KLVERSION \"%s\"", version))
print("#endif")

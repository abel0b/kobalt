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
    version = os.capture("ls doc/changelog | grep alpha | sed 's/.md//'")
end
    
print("#ifndef KBVERSION__H")
print("#define KBVERSION__H")
print(string.format("#define KBVERSION \"%s\"", version))
print("#endif")

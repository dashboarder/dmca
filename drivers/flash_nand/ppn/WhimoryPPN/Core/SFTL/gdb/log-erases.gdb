set logging redirect on
set logging overwrite on
set logging file ~/erases.txt
set logging on

set $idx = 0
set $max = sftl.__geom.max_sb

while ($idx < $max)
    set $t = sftl.sb[$idx].type
    set $v = sftl.sb[$idx].erases
    printf "%d:%d\n", $idx, $v
    set $idx = $idx + 1
end

set logging off
printf "Saved to ~/erases.txt; please analyze\n"

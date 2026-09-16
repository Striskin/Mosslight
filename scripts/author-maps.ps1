# Offline authoring utility. All coordinates below are handcrafted, not random.
# Runtime loads the resulting .map files directly. PowerShell is not a game dependency.
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
function New-Map([char]$floor, [char]$edge) {
    $script:grid = @()
    for ($y=0; $y -lt 34; $y++) {
        $line = ($floor.ToString() * 48).ToCharArray()
        for ($x=0; $x -lt 48; $x++) { if ($y -eq 0 -or $y -eq 33 -or $x -eq 0 -or $x -eq 47) { $line[$x]=$edge } }
        $script:grid += ,$line
    }
}
function Box([int]$x,[int]$y,[int]$w,[int]$h,[char]$tile) {
    for ($j=$y; $j -lt $y+$h; $j++) { for ($i=$x; $i -lt $x+$w; $i++) { $script:grid[$j][$i]=$tile } }
}
function Write-Map([string]$id,[int]$palette,[string]$name,[string]$subtitle,[string]$records) {
    $lines = @("MOSSMAP1 48 34 $palette `"$name`" `"$subtitle`"")
    $lines += $script:grid | ForEach-Object { -join $_ }
    $lines += $records.Trim()
    [IO.File]::WriteAllLines((Join-Path $root "data/maps/$id.map"),$lines,[Text.UTF8Encoding]::new($false))
}
New-Map '.' 'T'
Box 1 1 46 3 'T'; Box 1 28 12 5 'T'; Box 37 2 10 8 'T'; Box 2 4 3 12 'T'
Box 6 7 7 5 'H'; Box 31 7 7 5 'H'; Box 7 22 6 4 'H'
Box 18 14 8 8 '+'; Box 10 16 38 4 '+'; Box 20 7 3 19 '+'
Box 9 12 3 7 '+'; Box 33 12 3 7 '+'; Box 9 18 3 4 '+'
Box 31 23 13 8 '~'; Box 29 25 2 4 '~'; Box 34 21 7 2 '~'
Box 21 25 4 4 ','; Box 6 15 2 2 ','; Box 26 8 3 2 ','; Box 26 21 3 3 ','
Box 14 7 2 3 ','; Box 6 20 2 2 ','; Box 39 12 3 2 ','
Write-Map 'village' 0 'Hearthmere' 'Where every road begins with a warm cup' @'
exit 46 16 2 4 1 3 17 0
object 0 keeper 22 14 0 1 "Keeper Aven"
object 0 weaver 12 21 0 1 "Talla the Weaver"
object 0 guard 36 18 0 1 "Sir Rowan, roadwarden"
door smith_door 34 12 6 23 26 "The Ember & Anvil"
door inn_door 9 12 7 23 26 "The Copper Kettle"
object 2 village_lantern 19 17 0 1 "Hearth lantern"
object 1 village_supplies 10 13 0 2 "A traveller's welcome"
object 4 village_east 40 15 0 1 "EAST: Fernwake Wood, then Greywatch Bailey. The northeast house is the smith. The northwest house is the inn. Stand by a door and press E to enter."
object 4 village_water 29 23 0 1 "WASD: move. J: attack. L: heavy strike. K: guard. SPACE: dodge. 1/2/3: fists/sword/bow. Hold RMB to aim. E searches fallen enemies. Resting resets encounters."
'@
New-Map '.' 'T'
Box 1 1 16 5 'T'; Box 29 1 18 5 'T'; Box 1 24 14 9 'T'; Box 33 25 14 8 'T'
Box 8 11 7 4 'T'; Box 30 10 7 5 'T'; Box 9 20 5 3 'T'; Box 34 19 9 3 'T'
Box 1 16 30 4 '+'; Box 21 0 5 34 '+'; Box 28 16 15 3 '+'
Box 0 7 9 3 '+'; Box 6 8 3 9 '+'; Box 39 15 4 6 '+'
Box 0 16 2 4 '+'; Box 18 9 2 3 ','; Box 27 25 3 3 ','; Box 39 7 4 3 ','
Box 28 21 4 3 '~'; Box 29 24 4 3 '~'; Box 3 29 3 2 ','
Box 14 25 4 3 ','; Box 16 6 3 2 ','; Box 41 25 3 2 ','
Box 42 16 6 3 '+'
Write-Map 'forest' 1 'Fernwake Wood' 'Between birdsong and old, sleeping magic' @'
exit 0 16 2 4 0 43 17 0
exit 21 0 5 2 3 23 29 0
exit 21 32 5 2 2 23 3 0
exit 0 7 2 3 5 3 8 0
exit 46 16 2 3 8 3 17 0
object 1 forest_ember 41 17 4 1 "An ember-warm cache"
object 1 forest_tonic 17 27 0 2 "Herbalist's cache"
object 4 forest_crossroads 26 17 0 1 "NORTH: the shrine. SOUTH: Stillwater Cave. WEST: Hearthmere's shops. EAST: Greywatch Bailey, beware oathless knights. NORTHWEST: the Wandering Hollow and an old copper blade."
object 4 forest_hollow 5 10 0 1 "THE WANDERING HOLLOW. The stone paths shift for each traveller, then remember their footsteps. Copper waits in the far southeastern room."
enemy 0 17 17
enemy 0 36 17
enemy 1 25 10
enemy 0 23 26
enemy 1 39 24
enemy 4 11 17
enemy 4 17 8
enemy 4 30 28
'@
New-Map '#' '#'
Box 17 1 14 10 '.'; Box 8 9 31 9 '.'; Box 4 18 16 10 '.'; Box 25 19 18 11 '.'
Box 15 15 17 10 '.'; Box 21 0 5 5 '+'; Box 22 5 3 11 '+'
Box 9 14 29 3 '+'; Box 10 15 3 10 '+'; Box 28 16 3 10 '+'; Box 28 24 10 3 '+'
Box 17 9 3 3 '#'; Box 32 9 3 3 '#'; Box 21 19 3 3 '#'
Box 4 21 5 5 '~'; Box 9 24 8 3 '~'; Box 11 24 3 3 '_'
Box 35 21 5 2 '~'; Box 38 23 3 4 '~'; Box 17 3 2 3 '*'; Box 35 12 2 2 '*'
Box 6 18 2 2 '*'; Box 26 27 2 2 '*'
Write-Map 'cave' 2 'Stillwater Cave' 'Moonlight gathers where the water is quiet' @'
exit 21 0 5 2 1 23 29 0
object 1 cave_dew 35 26 5 1 "A moon-cooled cache"
object 1 cave_fragments 12 22 1 3 "Miner's satchel"
object 4 cave_note 26 4 0 1 "The water is deep. Stay on stone or timber. Lantern moths gather a bright spark before they cast it. Keep moving, then close the distance."
enemy 0 23 12
enemy 1 31 15
enemy 0 11 18
enemy 1 28 24
enemy 4 18 16
'@
New-Map '#' '#'
Box 10 7 28 23 ':'; Box 21 27 5 7 ':'
Box 12 8 6 1 's'; Box 31 8 5 1 's'; Box 31 10 4 3 'f'
Box 17 13 13 1 '='; Box 12 16 2 2 'b'; Box 34 19 2 2 'b'
Box 19 20 10 5 'r'; Box 28 9 1 1 'B'
Write-Map 'smithy' 4 'The Ember & Anvil' 'Steel, sinew, and a fair price' @'
exit 21 32 5 2 0 34 14 0
object 6 smith 23 15 0 1 "Master Harl, smith & trader"
object 4 smith_notes 29 23 0 1 "BUY with ENTER. Scroll with W/S. Sell shells and glow fragments for coins. Gear equips automatically; switch weapons with 1, 2, 3. Plate reduces heavy damage but makes you slower."
object 4 forge_notes 30 15 0 1 "A knight's lesson: your sword needs room. J is a quick cut, L a slower guard-breaking strike. Leave enough green stamina for a dodge. K raises your guard; time it well to stagger an attacker."
'@
New-Map '#' '#'
Box 9 7 30 23 '_'; Box 21 27 5 7 '_'
Box 11 8 6 1 's'; Box 30 9 6 1 '='; Box 35 8 2 1 'b'
Box 12 10 4 2 'f'; Box 13 17 3 2 '='; Box 30 20 4 2 '='
Box 11 24 3 2 'b'; Box 28 13 1 1 'B'; Box 19 19 7 6 'r'
Write-Map 'inn' 4 'The Copper Kettle' 'A warm hearth between uncertain roads' @'
exit 21 32 5 2 0 9 14 0
object 6 innkeeper 32 12 0 1 "Nessa, keeper of the Kettle"
object 2 inn_hearth 17 12 0 1 "The guest hearth"
object 4 inn_notice 26 25 0 1 "IRONBACK SHELLS WANTED. The smith pays six coins each. Defeat a beetle, approach its remains, and press E to search. Coins and unsold materials stay with you if you fall."
'@
New-Map '.' 'T'
Box 10 4 33 26 ':'; Box 9 4 2 26 '#'; Box 42 4 2 26 '#'
Box 9 4 35 2 '#'; Box 9 28 35 2 '#'; Box 0 16 20 4 '+'
Box 8 7 4 6 '#'; Box 8 22 4 6 '#'; Box 40 7 5 6 '#'; Box 40 22 5 6 '#'
Box 18 7 17 3 '#'; Box 20 9 13 1 '#'; Box 24 9 4 2 ':'
Box 15 11 2 2 'P'; Box 36 11 2 2 'P'; Box 15 23 2 2 'P'; Box 36 23 2 2 'P'
Box 16 9 1 1 'B'; Box 36 9 1 1 'B'; Box 16 25 1 1 'B'; Box 36 25 1 1 'B'
Box 31 23 3 2 'b'; Box 13 7 2 2 'b'; Box 34 14 2 2 'b'
Box 21 13 10 11 'r'; Box 23 25 6 2 ':'
Write-Map 'bailey' 4 'Greywatch Bailey' 'The old oath is written in steel' @'
exit 0 16 2 4 1 43 17 0
object 2 bailey_lantern 5 20 0 1 "Roadwarden's lantern"
object 0 guard 5 14 0 1 "Sir Elowen, the last watch"
object 1 bailey_plate 26 11 9 1 "The watch captain's armor"
object 1 bailey_arrows 37 26 8 15 "An archer's reserve"
object 4 bailey_warning 13 19 0 1 "OATHLESS KNIGHTS: shields stop frontal light attacks and arrows. Circle behind, use L to break guard, or time K just before a blow. Gold sparks mark an opening. Search the fallen with E."
enemy 3 22 18
enemy 3 33 18
enemy 3 26 13
enemy 4 18 25
'@
New-Map '.' 'T'
Box 1 1 16 5 'T'; Box 31 1 16 5 'T'; Box 1 25 13 8 'T'; Box 34 25 13 8 'T'
Box 11 7 26 18 ':'; Box 20 0 7 34 '+'; Box 14 14 20 5 ':'
Box 12 8 2 3 'P'; Box 34 8 2 3 'P'; Box 12 21 2 3 'P'; Box 34 21 2 3 'P'
Box 16 11 1 2 'P'; Box 30 11 1 2 'P'; Box 16 20 1 2 'P'; Box 30 20 1 2 'P'
Box 5 9 4 4 ','; Box 38 17 5 3 ','; Box 6 18 4 6 '~'
Write-Map 'shrine' 3 'The Quiet Bell Shrine' 'Two small lights can wake a forgotten road' @'
exit 20 32 7 2 1 23 4 0
exit 20 0 7 2 4 23 28 1
object 3 quiet_bowl 23 14 0 1 "The shrine bowl"
object 2 shrine_lantern 19 23 0 1 "Pilgrim lantern"
object 4 shrine_story 28 21 0 1 "We built no throne for the Warden. Only a garden, a bell, and a promise that someone would return."
object 1 shrine_tonics 32 17 0 2 "Pilgrim's offering"
'@
New-Map 'T' 'T'
for($y=3;$y -le 30;$y++) { for($x=5;$x -le 42;$x++) {
    if(([math]::Pow(($x-23.5)/19,2)+[math]::Pow(($y-16.5)/14,2)) -le 1) { $script:grid[$y][$x]='.' }
} }
Box 21 25 5 9 '+'; Box 17 11 14 12 ':'; Box 13 14 4 6 ':'; Box 31 14 4 6 ':'
Box 10 10 2 2 'P'; Box 35 10 2 2 'P'; Box 10 23 2 2 'P'; Box 35 23 2 2 'P'
Box 15 7 3 2 ','; Box 30 26 3 2 ','; Box 8 17 2 2 ','; Box 37 16 2 2 ','
Write-Map 'arena' 3 "The Warden's Garden" 'A guardian of roots, a heart of amber' @'
exit 21 32 5 2 3 23 4 0
enemy 2 23 15
object 4 arena_note 27 28 0 1 "Amber lines warn of a charge. An amber ring warns of scattered sparks. Dodge through danger. Strike during the quiet after. You can retreat south."
'@

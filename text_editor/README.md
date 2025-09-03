# C text editor.
Own implementation based on [antirez's kilo](http://antirez.com/news/108).
Compile with 
```sh
make editor
```
# Part 1
We read input from raw mode instead of canonical mode. For this we need to modify terminal's attributes using tcgetattr and tcsetattr. And unset some flags so we get our desired behaviour. All documented on code btw.


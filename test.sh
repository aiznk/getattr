echo "<h1>title</h1>" | ./getattr h1
echo "<h1 id=\"id\">title</h1>" | ./getattr h1 id
echo "<h1 id=\"id\" class=\"class\">title</h1>" | ./getattr h1 id class
echo "<h1 id=1 class=10>title</h1>" | ./getattr h1 id class
echo "<h1 id = 1 class = 10>title</h1>" | ./getattr h1 id class
echo "<h1 id = 1 class = 10>title</h1>" | ./getattr p id class

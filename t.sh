./main detect 12000
echo -n "test" | nc -u -w1 140.113.17.22 5000 -p 12000
echo listening
nc -lu -p 5000

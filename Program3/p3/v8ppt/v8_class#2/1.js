print("hi"); // not all args print
print('Hello World', 7, 9.9, true); // proper use
print(88, 8, 9.1, false); // first 88 becomes string... 
print(88, 8.99, 9.1, false); // 8.99 gets truncated 
print(88, 8, 9.1, 0); // expect false
print(88, 8, 9.1, 1); // expect true
print(88, 8, 9.1, "j"); // expect true
print(88, 8, 9.1, ""); // expect false
print(null, 8, 9.1, "j"); // expect null and true 
print(undefined, 8, 9.1, "j"); // expect undefined and true 
print("", 9);
print(); // return undefined
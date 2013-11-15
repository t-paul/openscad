//Test search with unicode strings

//"Normal" text for comparison
echo ("Lookup of 1 byte into 1 byte");
//Hits
echo("Expect [0]->",       search("a","aaaa") );
echo("Expect [0,1,2,3]->", search("a","aaaa",0) );
echo("Expect [0]->",       search("a","aaaa",1) );
echo("Expect [0,1]->",     search("a","aaaa",2) );
echo("Expect [0,1,2]->",   search("a","aaaa",3) );
echo("Expect [0,1,2,3]->", search("a","aaaa",4) );
//Misses
echo("Expect []->",        search("b","aaaa") );
echo("Expect []->",        search("b","aaaa",0) );
echo("Expect []->",        search("b","aaaa",1) );
echo("Expect []->",        search("b","aaaa",2) );

//Unicode tests
echo ("Lookup of multi-byte into 1 byte");
echo("Expect []->",        search("Л","aaaa") );
echo("Expect []->",        search("Л","aaaa",0) );
echo("Expect []->",        search("🂡","aaaa") );
echo("Expect []->",        search("🂡","aaaa",0) );

echo ("Lookup of 1-byte into multi-byte");
echo("Expect []->", search("a","ЛЛЛЛ") );
echo("Expect []->", search("a","🂡🂡🂡🂡") );
echo("Expect []->", search("a","ЛЛЛЛ",1) );
echo("Expect []->", search("a","🂡🂡🂡🂡",2) );

echo ("Lookup of 1-byte into mixed multi-byte");
echo("Expect [0,2,4,6,8]->", search("a","aЛaЛaЛaЛa",0) );
echo("Expect [0,2,4,6,8]->", search("a","a🂡a🂡a🂡a🂡a", 0) );
echo("Expect [0,4,8]->", search("a","a🂡Л🂡a🂡Л🂡a", 0) );

echo ("Lookup of 2-byte into 2-byte");
echo("Expect [0]->", search("Л","ЛЛЛЛ") );
echo("Expect [0,1,2,3]->", search("Л","ЛЛЛЛ",0) );

echo ("Lookup of 2-byte into 4-byte");
echo("Expect []->", search("Л","🂡🂡🂡🂡") );

echo ("Lookup of 4-byte into 4-byte");
echo("Expect [0]->", search("🂡","🂡🂡🂡🂡") );
echo("Expect [0,1,2,3]->", search("🂡","🂡🂡🂡🂡",0) );

echo ("Lookup of 4-byte into 2-byte");
echo("Expect [0]->", search("🂡","ЛЛЛЛ") );

echo ("Lookup of 2-byte into mixed multi-byte");
echo("Expect [0,2,4,6,8]->", search("Л","aЛaЛaЛaЛa",0) );
echo("Expect []->", search("Л","a🂡a🂡a🂡a🂡a", 0) );
echo("Expect [2,6]->", search("Л","a🂡Л🂡a🂡Л🂡a", 0) );

echo ("Lookup of 4-byte into mixed multi-byte");
echo("Expect []->", search("🂡","aЛaЛaЛaЛa",0) );
echo("Expect [1,3,5,7]->", search("🂡","a🂡a🂡a🂡a🂡a", 0) );
echo("Expect [1,3,5,7]->", search("🂡","a🂡Л🂡a🂡Л🂡a", 0) );

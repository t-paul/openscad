//Test how well arrays of inicode string are accessed.

texts_array = [
"DEADBEEF",
"Ленивый рыжий кот",
"كسول الزنجبيل القط"
,
"懶惰的姜貓",

"äöü ÄÖÜ ß",
"😁😂😃😄😅😆😇😈😉😊😋😌😍😎😏😐",
"⠁⠂⠃⠄⠅⠆⠇⠈⠉⠊⠋⠌⠍⠎⠏",
"🂡🂱🃁🃑",

];




for (text_array_idx = [0:(len(texts_array)-1)])
{
	echo( "Array element=", text_array_idx, "for", texts_array[text_array_idx], " of len=", len(texts_array[text_array_idx]), "[0]=", texts_array[text_array_idx][0],":"  );

for (text_idx = [0:(len(texts_array[text_array_idx])-1)])
{
	echo( "    [", text_idx, ,"]=", texts_array[text_array_idx][text_idx]  );
}


}


cube();

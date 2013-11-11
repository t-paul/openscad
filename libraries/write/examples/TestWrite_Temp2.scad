use <write/write.scad>


//writesphere("Ленивый рыжий кот" ,[0,0,-18],40.5,font="DejaVuSansCondensed",space=1.1);


write("Test                 ",font="DejaVuSansCondensed",space=1.1);



write("Лен",font="DejaVuSansCondensed",space=1.1);

text(t = "Лен", size = 16, font = "DejaVuSansCondensed");

t2 = "Лен";
t3 = "Л";
text(t = t2[0], size = 16, font = "DejaVuSansCondensed");
text(t = t3, size = 16, font = "DejaVuSansCondensed");
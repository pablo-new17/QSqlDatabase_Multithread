# QSqlDatabase_Multithread
Qt：QSqlDatabase的進一步封裝（多線程支持+更加簡單的操作）
-----
開發背景：

1.直接用QSqlDatabase我覺得太麻煩了；

2.對於某些數據庫，多個線程同時使用一個QSqlDatabase的時候會崩潰；

3.這段時間沒什麼乾貨放出來覺得渾身不舒服，就想寫一個。



於是，我就封裝了一下

只要簡單的實例化，然後通過query()就可以拿到QSqlQuery的實例化對象。

還自帶計時，一段時間不用可自動關閉，既保證效率也不佔用無用資源。



註：需要C++11的支持
————————————————
版权声明：本文为CSDN博主「Jason188080501」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/wsj18808050/article/details/44891715





-----
來源：https://blog.csdn.net/wsj18808050/article/details/44891715

void onConnection(conn) {
    int i = 0;
    while(i < 100) {
        conn->send("xxxxxxx");
        ++i;
        if(i == 50) {
            conn->shutdown();
        }
    }
}
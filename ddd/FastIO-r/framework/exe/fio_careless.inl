//	$Id: fio_careless.inl 2013-05-23 likunxiang$
//


int fio_set_txmsg(struct fio_nic *nic, struct fio_txdata *txdata);
//int handle_req(struct context *t, struct fio_rxdata *rxdata, struct fio_txdata *txdata)
//{
//    int port;
//    port = htons(*(u_short*)(rxdata->pbuf + g_sport_offset));
//    ND("tid %d: dsize %d port %d", t->me, rxdata->size, port); 
//    fio_setmsg_test(t, txdata); 
//    fio_set_txmsg(t, txdata);
//    return 0;
//}



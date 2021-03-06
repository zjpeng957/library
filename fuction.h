#include"client.h"
#include"book.h"
#include"global.h"
extern void ComputeCredit(Client *customer, unsigned int time);

extern int BorrowBook(Book *BorrowingBook, Client *customer, int *TotalBorrowedBook, unsigned int time);

extern bool ReturnBook(const unsigned long number, Client *customer, int *TotalBorrowedBook, Book *head,unsigned int time);

extern bool OrderBook(Book *BorrowingBook, Client *customer);

extern void OrderReminder(Client *customer, Book *head);

extern int OrdertoBorrow(const int decision, Client *customer, int *TotalBorrowedBook, Book *head,unsigned int time,int num);

extern void SearchBook(const char *name, int category, int press, const char *writer, unsigned long number, Book *FindingBook[], Book *head);

extern void SearchClient(QString name, unsigned int number, unsigned int id, int type, int school, Client *FindingClient[], Client *head);

extern double BorrowRate(int TotalBookNum, int TotalBorrowedBook);

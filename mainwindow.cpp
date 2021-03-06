#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include<fstream>
#include<iostream>
#include<vector>
#include<string>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	setStyleSheet("background-color:rgb(235, 242, 249)");
	Time.start();
	
	ui->stackedWidgetC->setCurrentIndex(0);
	ui->stackedWidgetA->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::ChoosePage(Client *CurClient)
{
	CurrentClient = CurClient;
	
	if (CurClient->type == ADMIN)
	{
		ui->stackedWidget->setCurrentIndex(0);
		AdminInit();
	}
	else
	{
		ui->stackedWidget->setCurrentIndex(1);
		
		ClientInit();
		OrderReminder(CurrentClient, BookInfo);
	}
	MessageInit();
}

//普通用户点击搜索按钮
void MainWindow::on_ButtonSearch_clicked()
{
	if (ui->stackedWidgetC->currentIndex() != 0)
	{
		ui->stackedWidgetC->setCurrentIndex(0);
	}
}
//普通用户点击还书按钮
void MainWindow::on_ButtonReturn_clicked()
{
	if (ui->stackedWidgetC->currentIndex() != 1)
	{
		ui->stackedWidgetC->setCurrentIndex(1);
	}
}
//普通用户点击查看个人信息按钮
void MainWindow::on_ButtonInfo_clicked()
{
	if (ui->stackedWidgetC->currentIndex() != 2)
	{
		ui->stackedWidgetC->setCurrentIndex(2);
	}
}

void MainWindow::on_ButtonHelpA_clicked(){}
void MainWindow::on_ButtonHelp_clicked()
{
	if (ui->stackedWidgetC->currentIndex() != 3)
	{
		ui->stackedWidgetC->setCurrentIndex(3);
	}
}

//管理员点击查找用户按钮
void MainWindow::on_ButtonSearchCA_clicked()
{
	if (ui->stackedWidgetA->currentIndex() != 1)
	{
		ui->stackedWidgetA->setCurrentIndex(1);
	}
}
//管理员点击购书按钮
void MainWindow::on_ButtonBuyA_clicked()
{
	if (ui->stackedWidgetA->currentIndex() != 2)
	{
		ui->stackedWidgetA->setCurrentIndex(2);
	}
}

//管理员点击查找书籍按钮
void MainWindow::on_ButtonSearchBA_clicked()
{
	if (ui->stackedWidgetA->currentIndex() != 0)
	{
		ui->stackedWidgetA->setCurrentIndex(0);
	}
}

//显示书籍搜索结果
void MainWindow::ShowSearchBook()
{
	//获取用户当前输入
	string BookName = qstr2str(ui->LineBookName->text());
	string BookWriter = qstr2str(ui->LineBookWriter->text());
	string number = qstr2str(ui->LineBookNumber->text());
	unsigned BookNumber = atoi(number.c_str());
	int BookCategory = ui->CBoxType->currentIndex();
	int BookPress = ui->CBoxPress->currentIndex();
	auto n = number != string() ? stoi(number) : 0;
	SearchBook(BookName.c_str(), BookCategory, BookPress, BookWriter.c_str(), n, SearchedBook, BookInfo);
	
	ui->tableResult->clearContents();
	ui->tableResult->setRowCount(0);
	for (int i = 0; SearchedBook[i]!=nullptr; i++)
	{
		ui->tableResult->setRowCount(i + 1);
		ui->tableResult->setItem(i, 0, new QTableWidgetItem(SearchedBook[i]->name));
		ui->tableResult->setItem(i, 1, new QTableWidgetItem(SearchedBook[i]->writer));
		ui->tableResult->setItem(i, 2, new QTableWidgetItem(str2qstr(press[SearchedBook[i]->press])));
		ui->tableResult->setItem(i, 3, new QTableWidgetItem(str2qstr(category[SearchedBook[i]->category])));
		ui->tableResult->setItem(i, 4, new QTableWidgetItem(QString::number(SearchedBook[i]->currentNumber)));
	}
}
//显示查找到的用户
void MainWindow::ShowSearchClient()
{
	QString name = ui->lineName->text();
	string id = qstr2str(ui->lineEditNo->text());
	string number = qstr2str(ui->lineCId->text());
	unsigned s = ui->comboBoxSchool->currentIndex();
	unsigned type = ui->comboBoxType->currentIndex();
	auto n = number == string() ? 0 : stoi(number);
	auto i = id == string() ? 0 : stoi(id);
	
	SearchClient(name, n, i, type,s,SearchedClient, ClientInfo);
	ui->tableResult->clearContents();
	ui->tableClient->setRowCount(0);
	for (size_t i = 0; i < MAX_C&&SearchedClient[i]; i++)
	{
		ui->tableClient->setRowCount(ui->tableClient->rowCount() + 1);
		ui->tableClient->setItem(i, 0, new QTableWidgetItem(SearchedClient[i]->name));
		ui->tableClient->setItem(i, 1, new QTableWidgetItem(str2qstr(to_string(SearchedClient[i]->number))));
		ui->tableClient->setItem(i, 2, new QTableWidgetItem(str2qstr(SearchedClient[i]->type==STUDENT?"学生":"教师")));
		ui->tableClient->setItem(i, 3, new QTableWidgetItem(str2qstr(school[SearchedClient[i]->school])));
	}
	QObject::connect(ui->tableClient, &QTableWidget::itemClicked, this, &MainWindow::ShowClientDetail,Qt::UniqueConnection);
}

//收到借书、预约或删除指令
void MainWindow::ChangeBook(int row, DetailType type)
{	
	switch (type)
	{
	case TBorrow://借阅书籍
		if (BorrowBook(SearchedBook[row], CurrentClient, &totalBorrowedBook, day))
		{
			BList.BorrowBook(CurrentClient->number, qstr2str(SearchedBook[row]->name), day);
			QMessageBox::information(this, str2qstr("提示"), str2qstr("借书成功！"), QMessageBox::Ok);
			ClientInfoInit();
			ClientReturnInit();
			ui->tableResult->setItem(row, 4, new QTableWidgetItem(QString::number(SearchedBook[row]->currentNumber)));
		}
		else QMessageBox::information(this, str2qstr("提示"), str2qstr("借书失败！"), QMessageBox::Ok);
		break;
	case TOrder://预约书籍
		if (OrderBook(SearchedBook[row],CurrentClient))
		{
			QMessageBox::information(this, str2qstr("提示"), str2qstr("预约成功！"), QMessageBox::Ok);
			ClientInfoInit();
		}
		else QMessageBox::information(this, str2qstr("提示"), str2qstr("预约失败！"), QMessageBox::Ok);
		break;
	case TDelete:
				db = new DBook(SearchedBook[row]);
				db->show();
				QObject::connect(db, &DBook::DeleteNo, this, &MainWindow::DeleteBook);
		break;
	default:
		break;
	}
}
//显示消息
void MainWindow::ShowMsg()
{
	msg = new msgWidget;
	
	for (int i = 0; i < MAX_B; i++)
	{
		if (CurrentClient->message[i] != 0)
		{
			msg->addMsg(i, CurrentClient->OrderedBook[i]);
		}
	}
	QObject::connect(msg, &msgWidget::msgClicked, this, &MainWindow::DealMsgClicked);
	msg->setAttribute(Qt::WA_DeleteOnClose);
	msg->show();
}
//删除用户
void MainWindow::DeleteClient()
{
	auto r=ui->tableClient->currentRow();
	cancel_client(SearchedClient[r]->id);
	ui->tableClient->removeRow(r);
}
void MainWindow::DeleteBook(unsigned n)
{
	auto result=cancel_book(n, day);
	if(result==1)
	{
		QMessageBox::information(this, str2qstr("提示"), str2qstr("删除成功！"), QMessageBox::Ok);
	}
	else QMessageBox::information(this, str2qstr("提示"), str2qstr("删除失败！"), QMessageBox::Ok);
}

//用户点击信息处理
void MainWindow::DealMsgClicked(bool decision,int index)
{
	OrdertoBorrow(decision, CurrentClient, &totalBorrowedBook, BookInfo, day, index);
	ClientInfoInit();
	ClientReturnInit();
	bool flag = true;
	CurrentClient->message[index] = 0;
	for (int i = 0; i < MAX_B&&flag; i++)
	{
		if (CurrentClient->message[i] != 0)
		{
			flag = false;
		}
	}
	if (flag)
	{
		QIcon icon("msg_void.png");
		msgAction->setIcon(icon);
	}
}
//10秒无事件则加一天
void MainWindow::mousePressEvent(QMouseEvent * event)
{
	int duration = Time.restart();
	if (duration > 10000) day += duration / 10000;
}

//用户界面初始化
void MainWindow::ClientInit()
{
	ui->ButtonSearch->setStyleSheet(SideButtonStyle);
	ui->ButtonReturn->setStyleSheet(SideButtonStyle);
	ui->ButtonInfo->setStyleSheet(SideButtonStyle);
	ui->label->setStyleSheet("background-color: rgb(60, 195, 245)");
	ui->ButtonSearch->setFocus();

	ClientSearchInit();
	ClientReturnInit();
	
	ClientInfoInit();
	
}
//还书确认
void MainWindow::ReturnBorrowedBook(QTableWidgetItem * Item)
{
	
	auto reply = QMessageBox::question(this, str2qstr("提示"), str2qstr("确认归还？"), QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		int i = 0,j=0;
		while (CurrentClient->bookNo[j] == 0) j++;
		while(i!=Item->row())
		{
			j++;
			while (CurrentClient->bookNo[j] == 0) j++;
			i++;
		}
		string n;
		
		auto result=ReturnBook(CurrentClient->bookNo[j], CurrentClient, &totalBorrowedBook, BookInfo, day,n);
		if (result&&(day - CurrentClient->time[j] > CurrentClient->authority))
		{
			string money = to_string(day - CurrentClient->time[j]);
			QMessageBox::information(this, str2qstr("提示"), str2qstr("借书超时，需支付"+money+"元"), QMessageBox::Ok);
		}
		BList.returnBook(CurrentClient->number, n, day);
		ClientInfoInit();
		//ClientReturnInit();
		ui->tableReturn->removeRow(i);

	}
}

void MainWindow::ShowBorrowRate()
{
	double rate;
	if (totalBook == 0) rate = 0;
	else rate = BorrowRate(totalBook, totalBorrowedBook);
	string r = to_string(rate);
	QMessageBox::about(this, str2qstr("借阅率"), str2qstr(r));
}

//用户查询界面初始化
void MainWindow::ClientSearchInit()
{
	ui->tableResult->setStyleSheet(OddTableUnitStyle);
	ui->tableResult->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableResult->setShowGrid(false);
	ui->tableResult->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	if (CurrentClient->type != ADMIN) ui->ButtonRate->hide();
	//书籍类型复选框初始化
	for (auto s : category)
	{
		ui->CBoxType->addItem(str2qstr(s.second));
	}

	//出版社复选框初始化
	for (auto s : press)
	{
		ui->CBoxPress->addItem(str2qstr(s.second));
	}

	//按钮初始化
	ui->ButtonConfirm->setStyleSheet(NormalButtonStyle);
	
	QObject::connect(ui->ButtonConfirm, &QPushButton::clicked, this, &MainWindow::ShowSearchBook);
	QObject::connect(ui->tableResult, &QTableWidget::itemClicked, this, &MainWindow::ShowBookDtail);
	QObject::connect(ui->ButtonRate, &QPushButton::clicked, this, &MainWindow::ShowBorrowRate);
}
//还书界面初始化
void MainWindow::ClientReturnInit()
{
	ui->tableReturn->setStyleSheet(OddTableUnitStyle);
	ui->tableReturn->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableReturn->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableReturn->setShowGrid(false);
	ui->tableReturn->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableReturn->setRowCount(0);
	for (int i = 0; i < MAX_B; i++)
	{
		//if (CurrentClient->bookNo[i] != 0)
		{
			ui->tableReturn->setRowCount(ui->tableReturn->rowCount() + 1);
			
			ui->tableReturn->setItem(i, 0, new QTableWidgetItem(CurrentClient->BorrowedBook[i]));
			ui->tableReturn->setItem(i, 1, new QTableWidgetItem(str2qstr(to_string(CurrentClient->bookNo[i]))));
			ui->tableReturn->setItem(i, 2, new QTableWidgetItem(str2qstr(to_string(CurrentClient->time[i]))));
			ui->tableReturn->setItem(i, 3, new QTableWidgetItem(str2qstr(to_string(MAX_DAYS - int(day-CurrentClient->time[i])))));
		}
	}
	QTableWidgetItem *p = nullptr;
	for (int i = 0; i < ui->tableReturn->rowCount(); i++)
	{
		p = ui->tableReturn->item(i, 0);
		if (p->text() == "aaa")
		{
			ui->tableReturn->removeRow(i);
			i--;
		}
	}

	QObject::connect(ui->tableReturn, &QTableWidget::itemClicked, this, &MainWindow::ReturnBorrowedBook,Qt::UniqueConnection);
}
//用户信息界面初始化
void MainWindow::ClientInfoInit()
{
	
	ui->stackedWidgetC->removeWidget(ui->PageInfo);
	ui->stackedWidgetC->insertWidget(2, new ClientDetail(CurrentClient));
}
//消息界面初始化
void MainWindow::MessageInit()
{
	QWidget *spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->toolBar->addWidget(spacer);

	if (CurrentClient->type != ADMIN)
	{
		int flag = 0;
		for (int i = 0; i < MAX_B; i++)
		{
			if (CurrentClient->message[i]) flag = 1;
		}

		if (flag) msgAction = new QAction(QIcon("msg_in.png"), "new", this);
		else msgAction = new QAction(QIcon("msg_void.png"), "new", this);

		ui->toolBar->addAction(msgAction);
		QObject::connect(msgAction, &QAction::triggered, this, &MainWindow::ShowMsg);
	}
	QAction *quitAction= new QAction(QIcon("quit.png"), "new", this);
	
	ui->toolBar->addAction(quitAction);
	
	QObject::connect(quitAction, &QAction::triggered, this, &MainWindow::close);
}

//管理员界面初始化
void MainWindow::AdminInit()
{
	ui->ButtonSearchBA->setStyleSheet(SideButtonStyle);
	ui->ButtonSearchCA->setStyleSheet(SideButtonStyle);
	ui->ButtonBuyA->setStyleSheet(SideButtonStyle);
	ui->ButtonBListA->setStyleSheet(SideButtonStyle);
	ui->label_2->setStyleSheet("background-color: rgb(60, 195, 245)");
	ui->ButtonSearchBA->setFocus();
	QObject::connect(ui->ButtonBListA, &QPushButton::clicked, this, &MainWindow::AdminListInit);
	ui->stackedWidgetA->removeWidget(ui->PageSearchBA);
	ui->stackedWidgetA->insertWidget(0, ui->PageSearch);
	ClientSearchInit();
	AdminSearchCInit();
	AdminBuyInit();
	ui->stackedWidgetA->setCurrentIndex(0);

}
//搜索用户界面初始化
void MainWindow::AdminSearchCInit()
{
	ui->tableClient->setStyleSheet(OddTableUnitStyle);
	ui->tableClient->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableClient->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableClient->setShowGrid(false);
	ui->tableClient->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	ui->comboBoxType->addItem(str2qstr("学生"));
	ui->comboBoxType->addItem(str2qstr("教师"));

	for (auto s : school)
	{
		ui->comboBoxSchool->addItem(str2qstr(s.second));
	}
	ui->ButtonClientSearch->setStyleSheet(NormalButtonStyle);
	QObject::connect(ui->ButtonClientSearch, &QPushButton::clicked, this, &MainWindow::ShowSearchClient);
}
//管理员输入新书信息，点击确认后的处理
void MainWindow::BuyBook()
{
	QString name = ui->lineBuyName->text();
	QString writer = ui->lineBuyAuthor->text();
	int press = ui->BoxBuyPress->currentIndex();
	int number= stoi(qstr2str(ui->lineBuyAmount->text()));
	int type = ui->BoxBuyCategory->currentIndex();
	double price = stof(qstr2str(ui->lineBuyPrice->text()));
	//----------------将输入的书籍信息传递给购书函数
	if (buy_book(name, writer, press, type, number, price, day) == 0)
	{
		QMessageBox::about(this, str2qstr("提示"), str2qstr("购入失败！"));
		return;
	}
	QMessageBox::about(this, str2qstr("提示"), str2qstr("购入成功！"));
	//将输入清空
	ui->lineBuyName->clear();
	ui->lineBuyAuthor->clear();
	ui->lineBuyAmount->clear();
	ui->lineBuyPrice->clear();
	ui->BoxBuyPress->setCurrentIndex(0);
	ui->BoxBuyCategory->setCurrentIndex(0);
}
//购书界面初始化
void MainWindow::AdminBuyInit()
{
	for (auto s : category)
	{
		ui->BoxBuyCategory->addItem(str2qstr(s.second));
	}
	for (auto s : press)
	{
		ui->BoxBuyPress->addItem(str2qstr(s.second));
	}

	QObject::connect(ui->pushButtonBuyConfirm, &QPushButton::clicked, this, &MainWindow::BuyBook);
}

void MainWindow::AdminListInit()
{
	ui->stackedWidgetA->setCurrentWidget(ui->PageList);
	ui->tableBList->setStyleSheet(OddTableUnitStyle);
	ui->tableBList->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableBList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableBList->setShowGrid(false);
	ui->tableBList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->tableBList->setRowCount(0);
	for (int i = 0; i < BList.ClientNo.size(); i++)
	{
		{
			ui->tableBList->setRowCount(ui->tableBList->rowCount() + 1);

			ui->tableBList->setItem(i, 0, new QTableWidgetItem(str2qstr(to_string(BList.ClientNo[i]))));
			ui->tableBList->setItem(i, 1, new QTableWidgetItem(str2qstr(BList.BookName[i])));
			ui->tableBList->setItem(i, 2, new QTableWidgetItem(str2qstr(to_string(BList.BorrowTime[i]))));
			ui->tableBList->setItem(i, 3, new QTableWidgetItem(str2qstr(to_string(BList.ReturnTime[i]))));
		}
	}
	/*QTableWidgetItem *p = nullptr;
	for (int i = 0; i < ui->tableReturn->rowCount(); i++)
	{
		p = ui->tableReturn->item(i, 0);
		if (p->text() == "aaa")
		{
			ui->tableReturn->removeRow(i);
			i--;
		}
	}*/

	//QObject::connect(ui->tableReturn, &QTableWidget::itemClicked, this, &MainWindow::ReturnBorrowedBook, Qt::UniqueConnection);
	
}

//点击书籍列表中的某一行后显示书籍详情
void MainWindow::ShowBookDtail(QTableWidgetItem * Item)
{
	QTableWidget *CurrentWidget = (QTableWidget *)sender();
	DetailType t;
	Book *TargetBook = SearchedBook[Item->row()];

	if (CurrentClient->type == ADMIN) t = TDelete;
	//else if (CurrentWidget == ui->tabBorrowed) t = TReturn;
	else if (CurrentWidget == ui->tableResult)
	{ 
		if (TargetBook->currentNumber > 0) t = TBorrow;
		else t = TOrder;
	}

	BookDetail *BD = new BookDetail(TargetBook, Item->row(), t);
	BD->setAttribute(Qt::WA_DeleteOnClose);
	BD->show();
	QObject::connect(BD, &BookDetail::changeItem, this, &MainWindow::ChangeBook);
}
//显示用户详细信息
void MainWindow::ShowClientDetail(QTableWidgetItem * Item)
{
	Client *TargetClient = SearchedClient[Item->row()];
	
	ClientDetail *CD = new ClientDetail(TargetClient);
	CD->setAttribute(Qt::WA_DeleteOnClose);
	CD->show();
	
	QObject::connect(CD, &ClientDetail::deleted, this, &MainWindow::DeleteClient);
}




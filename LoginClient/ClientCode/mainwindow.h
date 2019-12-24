#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Req_Login(QString _opt); //login process
    void Req_Login(QString _opt , QString _room_no); //join process

private slots:
    void on_Btn_Login_clicked();
    void on_login_reply();
    void on_Btn_Reg_clicked();
    void on_Btn_Join_clicked();

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager m_NetMng;

    QNetworkReply * m_pReply;

    enum login_opt
    {
        create_room,
        follow_room,
        change_password
    } m_cur_opt;


};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QFileDialog>
#include<QProcess>
#include<QMessageBox>
#include<QIntValidator>
#include<QDir>
#include<QString>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    QProcess* ssh_process;
    QString std_output;
    int selected;

private slots:
    void generate_clicked();
    void type_changed( int type );
    void select_file();
    void std_out();
    void finished( int exit_code );
};

#endif // MAINWINDOW_H

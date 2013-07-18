#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), selected( 0 )
{
    //create necessary slot/signal connections
    ui->setupUi(this);
    connect( ui->cancel_but,SIGNAL( clicked() ),this,SLOT(close()) );
    connect( ui->generate_but,SIGNAL( clicked() ),this,SLOT( generate_clicked() ) );
    connect( ui->browse_but,SIGNAL( clicked() ),this,SLOT( select_file() ) );
    connect( ui->typeBox,SIGNAL( currentIndexChanged(int) ),this,SLOT( type_changed( int ) ) );

    //fill combobox
    QStringList type_list;
    type_list.append( "rsa" );
    type_list.append( "dsa" );
    ui->typeBox->addItems( type_list );

    //etc
    ui->pass_edit->setEchoMode( QLineEdit::Password );
    ui->confirm_edit->setEchoMode( QLineEdit::Password );

    ui->file_edit->setText( QDir::homePath() + "/.ssh/id_key" );

    ui->bit_edit->setValidator( new QIntValidator( 256,8192,this ) );
}

MainWindow::~MainWindow()
{
    if( ssh_process->state() == QProcess::Running ){
        ssh_process->kill();
    }

    delete ui;
}

void MainWindow::generate_clicked(){

    ui->std_console->clear();

    ssh_process = new QProcess( this );
    QStringList* argument_list = new QStringList();

    *argument_list << "-t" << ui->typeBox->itemText( ui->typeBox->currentIndex() );

    if( !ui->file_edit->text().isEmpty() ){
        *argument_list << "-f" << ui->file_edit->text();
    }

    if( !ui->bit_edit->text().isEmpty() ){
        *argument_list << "-b" << ui->bit_edit->text();
    }

    if( !ui->pass_edit->text().isEmpty() ){
        
        if( ui->pass_edit->text() != ui->confirm_edit->text() ){
            QMessageBox::information( this,"Passphrases didnt match","Please renter your passphrase",QMessageBox::Ok,QMessageBox::Cancel );
            return;
        }

        if( ui->pass_edit->text().length() < 4 ){
            QMessageBox::information( this,"Passphrase too short","Passphrase must be at least > 4",QMessageBox::Ok,QMessageBox::Cancel );
            return;
        }

        *argument_list << "-N" << ui->pass_edit->text();
    }
    
    if( !ui->comment_edit->text().isEmpty() ){
        *argument_list << "-C" << ui->comment_edit->text();
    }

    connect( ssh_process,SIGNAL( readyRead() ),this,SLOT( std_out() )  ); //when there some text in output,
    connect( ssh_process,SIGNAL( finished( int ) ),this,SLOT( finished( int ) ) );

    ui->generate_but->setDisabled( true ); //dont touch while working!!!

    ssh_process->start( "ssh-keygen",*argument_list );
    this->setFixedHeight( 447 );  //show console

    delete argument_list;
}

void MainWindow::std_out(){
    std_output = ssh_process->readAll(); //read ssh-keygen output

    ui->std_console->insertPlainText( std_output ); //show in "console"

    if( std_output.contains( "Overwrite (y/n)? " ) ){ //if file already exist
        if( !selected ){ //because fileSelect dialog quering overwrite, i check only when file isnt selected by fileDialog
             if( QMessageBox::warning( this,"Overwrite?","Overwrite existing file?",QMessageBox::Yes,QMessageBox::No ) == QMessageBox::No ){
                ssh_process->write( "n\n" );
                ssh_process->waitForBytesWritten();
                ui->std_console->insertPlainText( "n\n" );
                return;
            }
        }
        ssh_process->write( "y\n" );
        ssh_process->waitForBytesWritten();
        ui->std_console->insertPlainText( "y\n" );
    }

}

void MainWindow::finished( int exit_code ){
    selected = 0;
    ui->generate_but->setDisabled( false );

    ui->std_console->insertPlainText( "Process finished with code " + QString::number( exit_code ));
    std_output.clear();
    delete ssh_process;
}

void MainWindow::select_file(){
    QFileDialog* file_dialog = new QFileDialog( this );

    file_dialog->setAcceptMode( QFileDialog::AcceptSave );
    file_dialog->setNameFilter( "Any file(*.*)" );
    file_dialog->setFileMode( QFileDialog::AnyFile );
    file_dialog->setViewMode( QFileDialog::Detail );

    if( file_dialog->exec() )
        ui->file_edit->setText( file_dialog->selectedFiles()[0] );

    selected++;
    delete file_dialog;
}

void MainWindow::type_changed( int type ){
    if( type == 1 ){ //dsa
        ui->bit_edit->setText( "1024" );
        ui->bit_edit->setDisabled( true );
    }else{//rsa
        ui->bit_edit->setText( "2048" );
        ui->bit_edit->setEnabled( true );
    }
}

#include "recvtokenhistory.h"
#include "transactiontablemodel.h"
#include "ui_recvtokenhistory.h"
#include "log/log.h"

RecvTokenHistory::RecvTokenHistory(const QModelIndex &idx,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecvTokenHistory)
{
    ui->setupUi(this);
    QString name = idx.data(TransactionTableModel::eCoinType).toString();
    QString num = idx.data(TransactionTableModel::eCoinNum).toString();
    QString add = idx.data(TransactionTableModel::AddressRole).toString();
    QString add1 = idx.data(TransactionTableModel::ToAddress).toString();
    QString lang = idx.data(TransactionTableModel::LanguageRole).toString();
    QString m_txid = idx.data(TransactionTableModel::TxIDRole).toString();
    LOG_WRITE(LOG_INFO,"RecvTokenHistory++whatwhatwaaaa",m_txid.toStdString().c_str());

    QString m_status = idx.data(TransactionTableModel::InfoStatus).toString();
    QString m_time = idx.data(TransactionTableModel::InfoTime).toString();
    qint64 txfee = idx.data(TransactionTableModel::feeAmount).toLongLong();

    if(txfee != 0)
    {
        QString m_txfee = BitcoinUnits::formatWithUnit(BitcoinUnit::IPC, txfee, false, BitcoinUnits::separatorAlways);
        ui->feeLabel->show();
        ui->feelabel->show();
        ui->widget_3->show();
        ui->feeLabel->setText(m_txfee);
        ui->line_5->show();
    }else{
        ui->feeLabel->hide();
        ui->feelabel->hide();
        ui->widget_3->hide();
        ui->line_5->hide();
    }

    ui->namelabel->setText("+"+num+" "+name);
    ui->add_label->setText(add);
    ui->timelabel->setText(m_time);
    ui->infolabel->setText(m_status);
}

RecvTokenHistory::~RecvTokenHistory()
{
    delete ui;
}
void RecvTokenHistory::updateInfo(QString status)
{
    ui->infolabel->setText(status);
}
void RecvTokenHistory::showVisual(bool visual)
{
    if(visual)
    {
        setVisible(true);
    }
    else
    {
        setVisible(false);
    }
}

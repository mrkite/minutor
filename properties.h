#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QDialog>
#include <QMap>

namespace Ui {
class Properties;
}

class QTreeWidgetItem;

class Properties : public QDialog
{
    Q_OBJECT

public:
    explicit Properties(QWidget *parent = 0);
    ~Properties();

    void DisplayProperties(QVariant p);
    
protected:
    void CreateTree(QTreeWidgetItem *node, const QVariant& v);
    QString GetSummary(const QString& key, const QVariant& v);

    template <class IterableT>
    void ParseIterable(QTreeWidgetItem* node, const IterableT& seq);
    template <class IterableT>
    void ParseList(QTreeWidgetItem* node, const IterableT& seq);

private:
    Ui::Properties *ui;
    QMap<QString, QString> summary;
};

#endif // PROPERTIES_H

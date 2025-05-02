#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QSortFilterProxyModel>

class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = nullptr);

    void loadData();

    QStringList getInstruments() const;
    QStringList getInstrumentSerials(const QString &instrument) const;
    QStringList getProducts() const;
    QStringList getProductSerials(const QString &product) const;
    QStringList getInspectors() const;
    QStringList getAllInstrumentSerials() const;
    QStringList getAllProductSerials() const;
    QStringList getPoints(const QString &name) const;

    QString instrumentBySerial(const QString &serial) const;
    QString productBySerial(const QString &serial) const;

private:
    QMap<QString, QStringList> m_instruments;
    QMap<QString, QStringList> m_products;
    QMap<QString, QString> m_serialToInstrument;
    QMap<QString, QString> m_serialToProduct;
    QMap<QString, QStringList> m_points;
    QStringList m_inspectors;
    QStringList m_allInstrumentSerials;
    QStringList m_allProductSerials;
};

class ContainsFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit ContainsFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        return index.data().toString().contains(filterRegularExpression());
    }
};

#endif // DATAMANAGER_H

#include "datamanager.h"

#include <QSqlQuery>
#include <QDebug>

DataManager::DataManager(QObject *parent) : QObject(parent) {}

void DataManager::loadData()
{
    // Загрузка приборов
    QSqlQuery instrumentQuery("SELECT MDM.name, MD.device_serial FROM measuring_device MD \
        JOIN measurement_characteristics MC \
        ON MD.id_characteristics = MC.id_measurement_characteristics \
        JOIN measuring_device_model MDM \
        ON MC.id_measuring_device_model = MDM.id_measuring_device_model;");
    while (instrumentQuery.next()) {
        QString name = instrumentQuery.value(0).toString();
        QString serial = instrumentQuery.value(1).toString();
        m_instruments[name].append(serial);
        m_serialToInstrument[serial] = name;
        m_allInstrumentSerials.append(serial);
    }

    // Загрузка изделий
    QSqlQuery productQuery("SELECT PD.name, P.product_serial FROM product P\
        JOIN product_type PD \
        ON P.id_product_type = PD.id_product_type;");
    while (productQuery.next()) {
        QString name = productQuery.value(0).toString();
        QString serial = productQuery.value(1).toString();
        m_products[name].append(serial);
        m_serialToProduct[serial] = name;
        m_allProductSerials.append(serial);
    }

    // QSqlQuery inspectorQuery("SELECT second_name FROM employee");
    // while (inspectorQuery.next()) {
    //     m_inspectors.append(inspectorQuery.value(0).toString());
    // }

    QSqlQuery query(
        "SELECT name || ' ' || second_name || COALESCE(' ' || middle_name, '') "
        "FROM employee");

    while (query.next()) {
        m_inspectors << query.value(0).toString();
    }
}

QStringList DataManager::getInstruments() const { return m_instruments.keys(); }

QStringList DataManager::getProducts() const { return m_products.keys(); }

QString DataManager::instrumentBySerial(const QString &serial) const
{
    return m_serialToInstrument.value(serial);
}

QString DataManager::productBySerial(const QString &serial) const
{
    return m_serialToProduct.value(serial);
}

QStringList DataManager::getInspectors() const { return m_inspectors; }

QStringList DataManager::getAllInstrumentSerials() const { return m_allInstrumentSerials; }
QStringList DataManager::getAllProductSerials() const { return m_allProductSerials; }

QStringList DataManager::getInstrumentSerials(const QString &instrument) const
{
    return instrument.isEmpty() ? m_allInstrumentSerials : m_instruments.value(instrument);
}

QStringList DataManager::getProductSerials(const QString &product) const
{
    return product.isEmpty() ? m_allProductSerials : m_products.value(product);
}

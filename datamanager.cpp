#include "datamanager.h"

#include <QSqlQuery>
#include <QDebug>

DataManager::DataManager(QObject *parent) : QObject(parent) {}

void DataManager::loadData()
{
    // Загрузка приборов
    QSqlQuery instrumentQuery("SELECT MDM.name, MD.device_code FROM protective_layer.measuring_device MD\
        JOIN measuring_device_model MDM \
        ON MD.id_device_model = MDM.id;");
    while (instrumentQuery.next()) {
        QString name = instrumentQuery.value(0).toString();
        QString serial = instrumentQuery.value(1).toString();
        m_instruments[name].append(serial);
        m_serialToInstrument[serial] = name;
        m_allInstrumentSerials.append(serial);
    }

    // Загрузка изделий
    QSqlQuery productQuery("SELECT PD.name, P.product_code FROM protective_layer.product P\
        JOIN product_type PD \
        ON P.id_product_type = PD.id;");
    while (productQuery.next()) {
        QString name = productQuery.value(0).toString();
        QString serial = productQuery.value(1).toString();
        m_products[name].append(serial);
        m_serialToProduct[serial] = name;
        m_allProductSerials.append(serial);
    }

    QSqlQuery inspectorQuery("SELECT second_name FROM examiner");
    while (inspectorQuery.next()) {
        m_inspectors.append(inspectorQuery.value(0).toString());
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

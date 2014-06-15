#include <QApplication>
#include <QDebug>
#include <QSslConfiguration>
#include <QCryptographicHash>

#include "webservice.h"
#include "logindialog.h"


//#define DEBUG

LhcWebServiceClient *LhcWebServiceClient::instance() {
static LhcWebServiceClient* fac = 0;
if (fac == 0 ) {
    fac = new LhcWebServiceClient();

    if (qApp != 0) {
        try {
           fac->setParent(qApp);
        }
        catch (...) {
            qDebug() << QString("%1 %2")
                        .arg("LHCWebServiceInstance::instance()")
                        .arg("failed to setParent");
        }
    }
}
return fac;
}

LhcWebServiceClient::LhcWebServiceClient()
{
    QSslConfiguration sslCfg = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> ca_list = sslCfg.caCertificates();
    QList<QSslCertificate> ca_new = QSslCertificate::fromData("CaCertificates");
    ca_list += ca_new;
    sslCfg.setCaCertificates(ca_list);
    sslCfg.setProtocol(QSsl::SslV3);
    QSslConfiguration::setDefaultConfiguration(sslCfg);

    URL = new QString();
    DomainURL = new QString();
    URLPostAddress = new QString();   

    #ifdef DEBUG
        qDebug("URL fetch constructor - %s", URL->toStdString().c_str());
    #endif
}


/**
* Sets main fetch url and headers
*/
void LhcWebServiceClient::setFetchURL(QString urlFetch, bool mode)
{
    *URL = urlFetch;
    QStringList lst( URL->split ("/") );
    QStringList::Iterator it2 = lst.begin();
    *DomainURL= *it2;
    *URLPostAddress = URL->replace(*DomainURL,"");
    *DomainURL = (mode == false ? QString("http://") : QString("https://"))+*DomainURL;

    #ifdef DEBUG
        qDebug("URL fetch assigned - %s", DomainURL->toStdString().c_str());
    #endif
}


QString LhcWebServiceClient::startRequest(QUrl url, bool executeCallback = false)
{
    QNetworkReply *reply = qnam.get(QNetworkRequest(url));
    if (executeCallback == true){
        reply->setProperty("url_request", QVariant(url.toString()));
        connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
    }

    return url.toString();
}

QString LhcWebServiceClient::startRequest(QUrl url, QUrl urlData, bool executeCallback = false)
{
    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));

    QNetworkReply *reply = qnam.post(request,urlData.toEncoded());

    QString hashString = (url.toString()+urlData.toString());
    QString requestMD5 = QString(QCryptographicHash::hash((hashString.toStdString().c_str()),QCryptographicHash::Md5).toHex());

    if (executeCallback == true){
        reply->setProperty("url_request", QVariant(requestMD5));
        connect(reply, SIGNAL(finished()), this, SLOT(requestFinished()));
    }

    return requestMD5;
}

void LhcWebServiceClient::LhcSendRequestAuthorization(QStringList query,QString address,QObject* pt2Object, void (*pt2Function)(void* pt2Object, QByteArray))
{

        QString searchStrin = query.join("&");

        OperationQueStruc reqstruc;
        reqstruc.pt2Function = pt2Function;
        reqstruc.pt2Object = pt2Object;

        #ifdef DEBUG
            qDebug("URL fetch assigned - %s", QUrl(*DomainURL+*URLPostAddress+"index.php"+address).toString().toStdString().c_str());
        #endif

        this->OperQuee.insert(this->startRequest(QUrl(*DomainURL+*URLPostAddress+"index.php"+address),QUrl(searchStrin),true), reqstruc);
}

/**
* Used for standard request with logins
*/
void LhcWebServiceClient::LhcSendRequest(QStringList query,QString address,QObject* pt2Object, void (*pt2Function)(void* pt2Object, QByteArray))
{    
    QString searchStrin = query.join("&")+"&username="+username+"&password="+password;

    OperationQueStruc reqstruc;
    reqstruc.pt2Function = pt2Function;
    reqstruc.pt2Object = pt2Object;

    this->OperQuee.insert(this->startRequest(QUrl(*DomainURL+*URLPostAddress+"index.php"+address),QUrl(searchStrin),true), reqstruc);
}

/**
* Request without parameters additional,
* @TODO:
* make GET from post.
*/
void LhcWebServiceClient::LhcSendRequest(QString address,QObject* pt2Object, void (*pt2Function)(void* pt2Object, QByteArray))
{
    QString searchStrin = "username="+QUrl::toPercentEncoding(username)+"&password="+QUrl::toPercentEncoding(password);

    OperationQueStruc reqstruc;
    reqstruc.pt2Function = pt2Function;
    reqstruc.pt2Object = pt2Object;

    this->OperQuee.insert(this->startRequest(QUrl(*DomainURL+*URLPostAddress+"index.php"+address),QUrl(searchStrin),true), reqstruc);
}

void LhcWebServiceClient::LhcSendRequest(QStringList query,QString address)
{
    QString searchString = query.join("&");
    searchString = searchString + "&username="+QUrl::toPercentEncoding(username)+"&password="+QUrl::toPercentEncoding(password);
    this->startRequest(QUrl(*DomainURL+*URLPostAddress+"index.php"+address),QUrl(searchString),false);
}

void LhcWebServiceClient::LhcSendRequest(QString address)
{
    QString auth = "username="+QUrl::toPercentEncoding(username)+"&password="+QUrl::toPercentEncoding(password);
    this->startRequest(QUrl(*DomainURL+*URLPostAddress+"index.php"+address),QUrl(auth),false);
}

void LhcWebServiceClient::requestFinished()
{

    #ifdef DEBUG
       // if (error == true)
            //qDebug("Could not connect - %s",QhttpClient->errorString().toStdString().c_str());
        //else
            //qDebug("Succesfuly connected - %s",QhttpClient->errorString().toStdString().c_str());
            //qDebug("Request finished %d",requestID);
            //qDebug("Request finished value %d",this->OperQuee.value(requestID));
            //qDebug("Queq size %d", error);
            //this->OperQuee.value(requestID);
    #endif

        QString requestID;
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

        if (reply) {
            if (reply->error() == QNetworkReply::NoError) {
                requestID = reply->property("url_request").toString();

                #ifdef DEBUG
                    qDebug("Finished request with ID - %s",requestID.toStdString().c_str());
                #endif

                if (!this->OperQuee.isEmpty())
                {
                    if (this->OperQuee.contains(requestID) && reply->error() == QNetworkReply::NoError)
                    {
                        QByteArray result = reply->readAll();
                        OperationQueStruc reqstruc = static_cast< OperationQueStruc > (this->OperQuee.take(requestID));

                        //  Associated with Quarded pointers,
                        // if some object destroyed before request finishes.
                        if (reqstruc.pt2Object)
                        reqstruc.pt2Function(reqstruc.pt2Object,result);

                    } else {
                        this->OperQuee.take(requestID);
                    }
                }
            };
            reply->deleteLater();
        }
}



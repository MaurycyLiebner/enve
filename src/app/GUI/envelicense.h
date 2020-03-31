#ifndef ENVELICENSE_H
#define ENVELICENSE_H
#include <QWidget>

class EnveLicense : public QWidget {
public:
    EnveLicense(QWidget* const parent);
    ~EnveLicense();

    static EnveLicense* sInstance;
private:
};

#endif // ENVELICENSE_H

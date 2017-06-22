#ifndef UPDATABLE_H
#define UPDATABLE_H


class Updatable {
public:
    Updatable();

    virtual void beforeUpdate() = 0;

    virtual void processUpdate() = 0;

    virtual void afterUpdate() = 0;
};

#endif // UPDATABLE_H

#ifndef ACTIONS_H
#define ACTIONS_H
class Document;
class Canvas;

class Actions {
public:
    Actions(Document& document);

private:
    Document& mDocument;
    Canvas* const & mActiveScene;
};

#endif // ACTIONS_H

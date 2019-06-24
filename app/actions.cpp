#include "actions.h"
#include "document.h"
#include "canvas.h"

Actions::Actions(Document &document) : mDocument(document),
    mActiveScene(mDocument.fActiveScene) {}

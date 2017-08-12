#include "mainwindow.h"


void MainWindow::createTablesInSaveDatabase(QSqlQuery *query) {
    query->exec("CREATE TABLE qrealanimator "
               "(id INTEGER PRIMARY KEY,"
               "currentvalue REAL )");

    query->exec("CREATE TABLE qrealkey "
               "(id INTEGER PRIMARY KEY, "
               "value REAL, "
               "frame INTEGER, "
               "endenabled BOOLEAN, "
               "startenabled BOOLEAN,"
               "ctrlsmode INTEGER, "
               "endvalue REAL, "
               "endframe INTEGER, "
               "startvalue REAL, "
               "startframe INTEGER, "
               "qrealanimatorid INTEGER, "
               "FOREIGN KEY(qrealanimatorid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE qstringanimator "
               "(id INTEGER PRIMARY KEY,"
               "currenttext TEXT )");

    query->exec("CREATE TABLE qstringkey "
               "(id INTEGER PRIMARY KEY, "
               "string TEXT, "
               "frame INTEGER, "
               "qstringanimatorid INTEGER, "
               "FOREIGN KEY(qstringanimatorid) REFERENCES qstringanimator(id) )");

    query->exec("CREATE TABLE transformanimator "
               "(id INTEGER PRIMARY KEY, "
               "posanimatorid INTEGER, "
               "scaleanimatorid INTEGER, "
               "pivotanimatorid INTEGER, "
               "rotanimatorid INTEGER, "
               "opacityanimatorid INTEGER, "
               "FOREIGN KEY(posanimatorid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(scaleanimatorid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(pivotanimatorid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(rotanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(opacityanimatorid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE qpointfanimator "
               "(id INTEGER PRIMARY KEY, "
               "xanimatorid INTEGER, "
               "yanimatorid INTEGER, "
               "FOREIGN KEY(xanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(yanimatorid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE coloranimator "
               "(id INTEGER PRIMARY KEY, "
               "colormode INTEGER, "
               "val1animatorid INTEGER, "
               "val2animatorid INTEGER, "
               "val3animatorid INTEGER, "
               "alphaanimatorid INTEGER, "
               "FOREIGN KEY(val1animatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(val2animatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(val3animatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(alphaanimatorid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE gradient "
               "(id INTEGER PRIMARY KEY)");

    query->exec("CREATE TABLE gradientpoints "
                "(id INTEGER PRIMARY KEY, "
                "endpointid INTEGER, "
                "startpointid INTEGER, "
                "FOREIGN KEY(endpointid) REFERENCES qpointfanimator(id), "
                "FOREIGN KEY(startpointid) REFERENCES qpointfanimator(id) )");

    query->exec("CREATE TABLE gradientcolor "
               "(colorid INTEGER, "
               "gradientid INTEGER, "
               "positioningradient INTEGER, "
               "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
               "FOREIGN KEY(gradientid) REFERENCES gradient(id) )");

    query->exec("CREATE TABLE paintsettings "
               "(id INTEGER PRIMARY KEY, "
               "painttype INTEGER, "
               "colorid INTEGER, "
               "gradientid INTEGER, "
               "gradientlinear BOOLEAN, "
               "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
               "FOREIGN KEY(gradientid) REFERENCES gradient(id) )");

    query->exec("CREATE TABLE strokesettings "
               "(id INTEGER PRIMARY KEY, "
               "linewidthanimatorid INTEGER, "
               "capstyle INTEGER, "
               "joinstyle INTEGER, "
               "paintsettingsid INTEGER, "
               "FOREIGN KEY(linewidthanimatorid) REFERENCES qrealanimator(id), "
               "FOREIGN KEY(paintsettingsid) REFERENCES paintsettings(id) )");


    query->exec("CREATE TABLE durationrect "
               "(id INTEGER PRIMARY KEY, "
                "minframe INTEGER, "
                "maxframe INTEGER )");


    query->exec("CREATE TABLE boundingbox "
               "(id INTEGER PRIMARY KEY, "
               "name TEXT, "
               "boxtype INTEGER, "
               "transformanimatorid INTEGER, "
               "pivotchanged BOOLEAN, "
               "visible BOOLEAN, "
               "locked BOOLEAN, "
               "blendmode INTEGER, "
               "parentboundingboxid INTEGER, "
               "durationrectid INTEGER, "
               "FOREIGN KEY(transformanimatorid) REFERENCES transformanimator(id), "
               "FOREIGN KEY(parentboundingboxid) REFERENCES boundingbox(id), "
               "FOREIGN KEY(durationrectid) REFERENCES durationrect(id) )");

    query->exec("CREATE TABLE canvas "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "width INTEGER, "
               "height INTEGER, "
               "framecount INTEGER, "
               "fps REAl, "
               "colorid INTEGER, "
               "cliptocanvas BOOLEAN, "
               "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE pathbox "
               "(id INTEGER PRIMARY KEY, "
               "fillgradientpointsid INTEGER, "
               "fillgradientendid INTEGER, "
               "strokegradientpointsid INTEGER, "
               "strokegradientendid INTEGER, "
               "boundingboxid INTEGER, "
               "fillsettingsid INTEGER, "
               "strokesettingsid INTEGER, "
               "FOREIGN KEY(fillgradientpointsid) REFERENCES gradientpoints(id), "
               "FOREIGN KEY(strokegradientpointsid) REFERENCES gradientpoints(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id), "
               "FOREIGN KEY(fillsettingsid) REFERENCES paintsettings(id), "
               "FOREIGN KEY(strokesettingsid) REFERENCES strokesettings(id) )");

    query->exec("CREATE TABLE pathpoint "
               "(id INTEGER PRIMARY KEY, "
               "isfirst BOOLEAN, "
               "isendpoint BOOLEAN, "
               "qpointfanimatorid INTEGER, "
               "startctrlptid INTEGER, "
               "endctrlptid INTEGER, "
               "boundingboxid INTEGER, "
               "ctrlsmode INTEGER, "
               "startpointenabled BOOLEAN, "
               "endpointenabled BOOLEAN, "
               "FOREIGN KEY(qpointfanimatorid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(startctrlptid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(endctrlptid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE circle "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "horizontalradiuspointid INTEGER, "
               "verticalradiuspointid INTEGER, "
               "FOREIGN KEY(horizontalradiuspointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(verticalradiuspointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE rectangle "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "topleftpointid INTEGER, "
               "bottomrightpointid INTEGER, "
               "radiuspointid INTEGER, "
               "FOREIGN KEY(topleftpointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(bottomrightpointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(radiuspointid) REFERENCES qpointfanimator(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE textbox "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "stringanimatorid INTEGER, "
               "fontfamily TEXT, "
               "fontstyle TEXT, "
               "fontsize REAL, "
               "FOREIGN KEY(stringanimatorid) REFERENCES qstringanimator(id), "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE imagebox "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "imagefilepath TEXT, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE videobox "
               "(id INTEGER PRIMARY KEY, "
               "boundingboxid INTEGER, "
               "srcfilepath TEXT, "
               "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE pixmapeffect "
                "(id INTEGER PRIMARY KEY, "
                "boundingboxid INTEGER, "
                "type INTEGER, "
                "FOREIGN KEY(boundingboxid) REFERENCES boundingbox(id) )");

    query->exec("CREATE TABLE blureffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "radiusid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(radiusid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE shadoweffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "blurradiusid INTEGER, "
                "colorid INTEGER, "
                "opacityid INTEGER, "
                "translationid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(blurradiusid) REFERENCES qrealanimator(id), "
                "FOREIGN KEY(colorid) REFERENCES coloranimator(id), "
                "FOREIGN KEY(opacityid) REFERENCES qrealanimator(id), "
                "FOREIGN KEY(translationid) REFERENCES qpointfanimator(id) )");

    query->exec("CREATE TABLE lineseffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "distanceid INTEGER, "
                "widthid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(distanceid) REFERENCES qrealanimator(id), "
                "FOREIGN KEY(widthid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE circleseffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "distanceid INTEGER, "
                "radiusid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(distanceid) REFERENCES qrealanimator(id), "
                "FOREIGN KEY(radiusid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE swirleffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "degreesid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(degreesid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE oileffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "radiusid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(radiusid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE implodeeffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "factorid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(factorid) REFERENCES qrealanimator(id) )");

    query->exec("CREATE TABLE desaturateeffect "
                "(id INTEGER PRIMARY KEY, "
                "pixmapeffectid INTEGER, "
                "influenceid INTEGER, "
                "FOREIGN KEY(pixmapeffectid) REFERENCES pixmapeffect(id), "
                "FOREIGN KEY(influenceid) REFERENCES qrealanimator(id) )");
}

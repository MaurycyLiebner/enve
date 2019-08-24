// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <exception>
#include <csignal>
#include <QDebug>
#define ERROUT(msg) qDebug() << msg << __LINE__
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef QT_DEBUG
// Breakpoint for QT_DEBUG
    #define RuntimeThrow(msg) \
    std::throw_with_nested(\
        std::runtime_error(\
            (std::raise(SIGTRAP) ? "" : "") + \
            std::to_string(__LINE__) + "  :  " + \
            FILENAME + "  :  " + __func__ + "()\n  " + msg\
        )\
    )
#else
    #define RuntimeThrow(msg) \
    std::throw_with_nested(\
        std::runtime_error(\
            std::to_string(__LINE__) + "  :  " + \
            FILENAME + "  :  " + __func__ + "()\n  " + msg\
        )\
    )
#endif

#define CheckInvalidLocation(vLoc, name) \
    if(vLoc < 0) { \
        checkGlErrors; \
        RuntimeThrow("Invalid " name " location."); \
    }

std::string operator+(const std::string& c, const QString& k);
std::string operator<<(const std::string& c, const QString& k);
std::string operator>>(const QString& k, const std::string& c);

QDebug operator<<(QDebug out, const std::string& str);

extern QString gAllTextFromException(const std::exception& e,
                                     QString allText = "",
                                     const uint level = 0);
extern void gPrintExceptionCritical(const std::exception& e);
extern void gPrintExceptionFatal(const std::exception& e);
extern void gPrintException(const bool fatal, const QString& allText);
extern void gPrintExceptionCritical(const std::exception_ptr& eptr);
extern void gPrintExceptionFatal(const std::exception_ptr& eptr);

#endif // EXCEPTIONS_H

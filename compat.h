// Copyright (c) 2026, Nick Korotysh
#pragma once

/*
 * This file contains various compatibility stuff
 * to support builds with both Qt 5 and Qt 6.
 *
 * It should be dropped with Qt 5 build support.
 */

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QString>
// QString::SplitBehavior is obsolete since Qt 5.14,
// and was completely removed in Qt 6.
namespace Qt {
// create aliases to emulate required values in old Qt
constexpr auto SkipEmptyParts = QString::SkipEmptyParts;
}
#endif

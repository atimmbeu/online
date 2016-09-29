/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Common.hpp"
#include "LOOLProtocol.hpp"
#include "MessageQueue.hpp"
#include "Util.hpp"

namespace CPPUNIT_NS
{
template<>
struct assertion_traits<std::vector<char>>
{
    static bool equal(const std::vector<char>& x, const std::vector<char>& y)
    {
        return x == y;
    }

    static std::string toString(const std::vector<char>& x)
    {
        const std::string text = '"' + (!x.empty() ? std::string(x.data(), x.size()) : "<empty>") + '"';
        return text;
    }
};
}

/// TileQueue unit-tests.
class TileQueueTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(TileQueueTests);

    CPPUNIT_TEST(testTileQueuePriority);
    CPPUNIT_TEST(testTileCombinedRendering);

    CPPUNIT_TEST_SUITE_END();

    void testTileQueuePriority();
    void testTileCombinedRendering();
};

void TileQueueTests::testTileQueuePriority()
{
    const std::string reqHigh = "tile part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840";
    const std::string resHigh = "tile part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840 ver=-1";
    const TileQueue::Payload payloadHigh(resHigh.data(), resHigh.data() + resHigh.size());
    const std::string reqLow = "tile part=0 width=256 height=256 tileposx=0 tileposy=253440 tilewidth=3840 tileheight=3840";
    const std::string resLow = "tile part=0 width=256 height=256 tileposx=0 tileposy=253440 tilewidth=3840 tileheight=3840 ver=-1";
    const TileQueue::Payload payloadLow(resLow.data(), resLow.data() + resLow.size());

    TileQueue queue;

    // Request the tiles.
    queue.put(reqLow);
    queue.put(reqHigh);

    // Original order.
    CPPUNIT_ASSERT_EQUAL(payloadLow, queue.get());
    CPPUNIT_ASSERT_EQUAL(payloadHigh, queue.get());

    // Request the tiles.
    queue.put(reqLow);
    queue.put(reqHigh);
    queue.put(reqHigh);
    queue.put(reqLow);

    // Set cursor above reqHigh.
    queue.updateCursorPosition(0, 0, 0, 0, 10, 100);

    // Prioritized order.
    CPPUNIT_ASSERT_EQUAL(payloadHigh, queue.get());
    CPPUNIT_ASSERT_EQUAL(payloadLow, queue.get());

    // Repeat with cursor position set.
    queue.put(reqLow);
    queue.put(reqHigh);
    CPPUNIT_ASSERT_EQUAL(payloadHigh, queue.get());
    CPPUNIT_ASSERT_EQUAL(payloadLow, queue.get());

    // Repeat by changing cursor position.
    queue.put(reqLow);
    queue.put(reqHigh);
    queue.updateCursorPosition(0, 0, 0, 253450, 10, 100);
    CPPUNIT_ASSERT_EQUAL(payloadLow, queue.get());
    CPPUNIT_ASSERT_EQUAL(payloadHigh, queue.get());
}

void TileQueueTests::testTileCombinedRendering()
{
    const std::string req1 = "tile part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840";
    const std::string req2 = "tile part=0 width=256 height=256 tileposx=3840 tileposy=0 tilewidth=3840 tileheight=3840";
    const std::string req3 = "tile part=0 width=256 height=256 tileposx=0 tileposy=3840 tilewidth=3840 tileheight=3840";
    const std::string req4 = "tile part=0 width=256 height=256 tileposx=3840 tileposy=3840 tilewidth=3840 tileheight=3840";

    const std::string resHor = "tilecombine part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 imgsize=0,0 tilewidth=3840 tileheight=3840";
    const TileQueue::Payload payloadHor(resHor.data(), resHor.data() + resHor.size());
    const std::string resVer = "tilecombine part=0 width=256 height=256 tileposx=0,0 tileposy=0,3840 imgsize=0,0 tilewidth=3840 tileheight=3840";
    const TileQueue::Payload payloadVer(resVer.data(), resVer.data() + resVer.size());
    const std::string resFull = "tilecombine part=0 width=256 height=256 tileposx=0,3840,0 tileposy=0,0,3840 imgsize=0,0,0 tilewidth=3840 tileheight=3840";
    const TileQueue::Payload payloadFull(resFull.data(), resFull.data() + resFull.size());

    TileQueue queue;

    // Horizontal.
    queue.put(req1);
    queue.put(req2);
    CPPUNIT_ASSERT_EQUAL(payloadHor, queue.get());

    // Vertical.
    queue.put(req1);
    queue.put(req3);
    CPPUNIT_ASSERT_EQUAL(payloadVer, queue.get());

    // Vertical.
    queue.put(req1);
    queue.put(req2);
    queue.put(req3);
    CPPUNIT_ASSERT_EQUAL(payloadFull, queue.get());
}

CPPUNIT_TEST_SUITE_REGISTRATION(TileQueueTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
//
//  toolIcon.js
//
//  Created by David Rowe on 28 Jul 2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

/* global ToolIcon */

ToolIcon = function (side) {
    // Tool icon displayed on non-dominant hand.

    "use strict";

    var SCALE_TOOL = 0,
        CLONE_TOOL = 1,
        GROUP_TOOL = 2,

        ICON_COLORS = [
            { red: 0, green: 240, blue: 240 },
            { red: 240, green: 0, blue: 240 },
            { red: 240, green: 240, blue: 0 }
        ],

        LEFT_HAND = 0,
        AVATAR_SELF_ID = "{00000000-0000-0000-0000-000000000001}",

        ICON_DIMENSIONS = { x: 0.1, y: 0.01, z: 0.1 },
        ICON_POSITION = { x: 0, y: 0.01, z: 0 },
        ICON_ROTATION = Quat.fromVec3Degrees({ x: 0, y: 0, z: 0 }),

        ICON_TYPE = "sphere",
        ICON_PROPERTIES = {
            dimensions: ICON_DIMENSIONS,
            localPosition: ICON_POSITION,
            localRotation: ICON_ROTATION,
            solid: true,
            alpha: 1.0,
            parentID: AVATAR_SELF_ID,
            ignoreRayIntersection: false,
            visible: true
        },

        handJointName,

        iconOverlay = null;

    if (!this instanceof ToolIcon) {
        return new ToolIcon();
    }

    function setHand(side) {
        // Assumes UI is not displaying.
        handJointName = side === LEFT_HAND ? "LeftHand" : "RightHand";
    }

    setHand(side);

    function update() {
        // TODO: Display icon animation.
        // TODO: Clear icon animation.
    }

    function display(icon) {
        // Displays icon on hand.
        var handJointIndex,
            iconProperties;

        handJointIndex = MyAvatar.getJointIndex(handJointName);
        if (handJointIndex === -1) {
            // Don't display if joint isn't available (yet) to attach to.
            // User can clear this condition by toggling the app off and back on once avatar finishes loading.
            // TODO: Log error.
            return;
        }

        if (iconOverlay === null) {
            iconProperties = Object.clone(ICON_PROPERTIES);
            iconProperties.parentJointIndex = handJointIndex;
            iconProperties.color = ICON_COLORS[icon];
            iconOverlay = Overlays.addOverlay(ICON_TYPE, iconProperties);
        } else {
            Overlays.editOverlay(iconOverlay, { color: ICON_COLORS[icon] });
        }
    }

    function clear() {
        // Deletes current icon.
        if (iconOverlay) {
            Overlays.deleteOverlay(iconOverlay);
            iconOverlay = null;
        }
    }

    function destroy() {
        clear();
    }

    return {
        SCALE_TOOL: SCALE_TOOL,
        CLONE_TOOL: CLONE_TOOL,
        GROUP_TOOL: GROUP_TOOL,
        setHand: setHand,
        update: update,
        display: display,
        clear: clear,
        destroy: destroy
    };
};

ToolIcon.prototype = {};

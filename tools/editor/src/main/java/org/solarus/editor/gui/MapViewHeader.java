/*
 * Copyright (C) 2006-2014 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus Quest Editor is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus Quest Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
package org.solarus.editor.gui;

import org.solarus.editor.Map;
import java.awt.*;
import javax.swing.*;

/**
 * This component shows the header of the map view.
 * It contains a toolbar to add new entities, and some options
 * to choose how the map is displayed.
 */
public class MapViewHeader extends JPanel {

    /**
     * Constructor.
     * @param map The map.
     * @param mapView The map view.
     * @param mapViewScroller The map view's scroller
     */
    MapViewHeader(Map map, MapView mapView, JScrollPane mapViewScroller) {
        super(new BorderLayout());

        MapViewSettingsPanel settingsPanel =
            new MapViewSettingsPanel(map.getViewSettings(), mapViewScroller);

        AddEntitiesToolbar addEntitiesToolbar = new AddEntitiesToolbar(mapView);

        add(settingsPanel, BorderLayout.CENTER);
        add(addEntitiesToolbar, BorderLayout.SOUTH);
    }
}


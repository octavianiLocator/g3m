

package planarviewer.client;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.RootPanel;


public class GPlanarViewerEntryPoint
         extends
            FlexTable
         implements
            EntryPoint {

   @Override
   public void onModuleLoad() {
      // TODO Auto-generated method stub
      System.out.println("> Starting up..");

      super.setCellPadding(0);
      super.setCellSpacing(0);
      super.setBorderWidth(0);
      //super.getRowFormatter().setVerticalAlign(1, HasVerticalAlignment.ALIGN_TOP);
      //VerticalAlignmentConstant x;

      GImageLoader.load("BostonCityFlow.jpg", new OnLoadHandler(0, 0));
      setWidget(0, 0, new GImage("BostonCityFlow.jpg"));

      GImageLoader.load("BostonCityFlow.jpg", new OnLoadHandler(0, 1) {
         @Override
         public void imageLoaded(final GImageLoadEvent event) {
            super.imageLoaded(event);
            setWidget(_row, _col, event.takeImage());
         }
      });

      GImageLoader.load("BostonCityFlow.jpg", new OnLoadHandler(1, 0));
      setWidget(1, 0, new GImage("BostonCityFlow.jpg"));

      GImageLoader.load("BostonCityFlow.jpg", new OnLoadHandler(1, 1));
      setWidget(1, 1, new GImage("BostonCityFlow.jpg"));

      RootPanel.get().add(this);
   }

   private class OnLoadHandler
            implements
               IImageLoadHandler {

      final int _row, _col;


      public OnLoadHandler(final int row,
                           final int col) {
         _row = row;
         _col = col;
      }


      @Override
      public void imageLoaded(final GImageLoadEvent event) {

         if (event.isLoadFailed()) {
            if (!isCellPresent(_row + 1, _col)) {
               //setText(_row + 1, _col, "Image failed to load.");
               GWT.log("Image failed to load.");
            }
         }
         else {
            //event.getImage().setFixedSize(1024, 683);
            //event.takeImage().setPixelSize(1024, 683);
            if (!isCellPresent(_row + 1, _col)) {
               //               setText(_row + 2, _col, "Image dimensions: " + event.getDimensions().getWidth() + " x "
               //                                       + event.getDimensions().getHeight());
               GWT.log("Image dimensions: " + event.getDimensions().getWidth() + " x " + event.getDimensions().getHeight());
            }
         }
      }
   }


}

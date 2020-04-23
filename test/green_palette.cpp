#include <gtest/gtest.h>
#include "cap32.h"

TEST(GreenPalette, ValuesMatchFormula)
{
   double precision = 0.0001;

   double G_LUMA_R =    0.2427;
   double G_LUMA_G =    0.6380;
   double G_LUMA_B =    0.1293;
   double G_LUMA_BASE = 0.071;
   double G_LUMA_COEF = 0.100;
   double G_LUMA_PRIM = 0.050;

   //test the new libretro palette (mode 1)
   double *green_palette = video_get_green_palette(1);

   for (int color = 0; color < 32; color++) {
	  double *colours_rgb = video_get_rgb_color(color);
	  double r = colours_rgb[0];
	  double g = colours_rgb[1];
	  double b = colours_rgb[2];

	  double green_luma = ((G_LUMA_R * r) + (G_LUMA_G * g) + (G_LUMA_B * b));
		 green_luma += (G_LUMA_BASE + G_LUMA_PRIM - (G_LUMA_COEF * green_luma));

      EXPECT_NEAR(green_luma, green_palette[color], precision) <<
    		  "green_luma value differs for " << color << ".";
   }
}


#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#define	HAL_major		0
#define	HAL_minor		9
#define	HAL_revision 	0

void set_display_direct ();
void set_display_normal (bool force=false);
extern fabgl::VGABaseController* display;
extern TaskHandle_t  mainTaskHandle;

#endif // __GLOBALS_H_
#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.xdg_document"), aligned (8)))
#else
# define SECTION
#endif

static const SECTION union { const guint8 data[2605]; const double alignment; void * const ptr;}  _xdg_document_resource_data = { {
  0x47, 0x56, 0x61, 0x72, 0x69, 0x61, 0x6e, 0x74, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x18, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x28, 0x06, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
  0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 
  0x77, 0xd2, 0xd3, 0x10, 0x04, 0x00, 0x00, 0x00, 
  0xc8, 0x00, 0x00, 0x00, 0x07, 0x00, 0x4c, 0x00, 
  0xd0, 0x00, 0x00, 0x00, 0xd4, 0x00, 0x00, 0x00, 
  0x4b, 0x50, 0x90, 0x0b, 0x02, 0x00, 0x00, 0x00, 
  0xd4, 0x00, 0x00, 0x00, 0x04, 0x00, 0x4c, 0x00, 
  0xd8, 0x00, 0x00, 0x00, 0xdc, 0x00, 0x00, 0x00, 
  0xd4, 0xb5, 0x02, 0x00, 0xff, 0xff, 0xff, 0xff, 
  0xdc, 0x00, 0x00, 0x00, 0x01, 0x00, 0x4c, 0x00, 
  0xe0, 0x00, 0x00, 0x00, 0xe4, 0x00, 0x00, 0x00, 
  0x38, 0x7c, 0x95, 0xa5, 0x00, 0x00, 0x00, 0x00, 
  0xe4, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x4c, 0x00, 
  0xf0, 0x00, 0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 
  0xd6, 0xfc, 0x67, 0x05, 0x01, 0x00, 0x00, 0x00, 
  0xf4, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x4c, 0x00, 
  0x00, 0x01, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 
  0x09, 0x22, 0xcf, 0x85, 0x03, 0x00, 0x00, 0x00, 
  0x04, 0x01, 0x00, 0x00, 0x24, 0x00, 0x76, 0x00, 
  0x28, 0x01, 0x00, 0x00, 0x2d, 0x0a, 0x00, 0x00, 
  0x70, 0x6f, 0x72, 0x74, 0x61, 0x6c, 0x2f, 0x00, 
  0x03, 0x00, 0x00, 0x00, 0x6f, 0x72, 0x67, 0x2f, 
  0x04, 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, 0x00, 
  0x01, 0x00, 0x00, 0x00, 0x44, 0x6f, 0x63, 0x75, 
  0x6d, 0x65, 0x6e, 0x74, 0x73, 0x2f, 0x00, 0x00, 
  0x05, 0x00, 0x00, 0x00, 0x66, 0x72, 0x65, 0x65, 
  0x64, 0x65, 0x73, 0x6b, 0x74, 0x6f, 0x70, 0x2f, 
  0x00, 0x00, 0x00, 0x00, 0x6f, 0x72, 0x67, 0x2e, 
  0x66, 0x72, 0x65, 0x65, 0x64, 0x65, 0x73, 0x6b, 
  0x74, 0x6f, 0x70, 0x2e, 0x70, 0x6f, 0x72, 0x74, 
  0x61, 0x6c, 0x2e, 0x44, 0x6f, 0x63, 0x75, 0x6d, 
  0x65, 0x6e, 0x74, 0x73, 0x2e, 0x78, 0x6d, 0x6c, 
  0xf5, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x3c, 0x21, 0x44, 0x4f, 0x43, 0x54, 0x59, 0x50, 
  0x45, 0x20, 0x6e, 0x6f, 0x64, 0x65, 0x20, 0x50, 
  0x55, 0x42, 0x4c, 0x49, 0x43, 0x0a, 0x22, 0x2d, 
  0x2f, 0x2f, 0x66, 0x72, 0x65, 0x65, 0x64, 0x65, 
  0x73, 0x6b, 0x74, 0x6f, 0x70, 0x2f, 0x2f, 0x44, 
  0x54, 0x44, 0x20, 0x44, 0x2d, 0x42, 0x55, 0x53, 
  0x20, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x20, 
  0x49, 0x6e, 0x74, 0x72, 0x6f, 0x73, 0x70, 0x65, 
  0x63, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x31, 0x2e, 
  0x30, 0x2f, 0x2f, 0x45, 0x4e, 0x22, 0x0a, 0x22, 
  0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 
  0x77, 0x77, 0x2e, 0x66, 0x72, 0x65, 0x65, 0x64, 
  0x65, 0x73, 0x6b, 0x74, 0x6f, 0x70, 0x2e, 0x6f, 
  0x72, 0x67, 0x2f, 0x73, 0x74, 0x61, 0x6e, 0x64, 
  0x61, 0x72, 0x64, 0x73, 0x2f, 0x64, 0x62, 0x75, 
  0x73, 0x2f, 0x31, 0x2e, 0x30, 0x2f, 0x69, 0x6e, 
  0x74, 0x72, 0x6f, 0x73, 0x70, 0x65, 0x63, 0x74, 
  0x2e, 0x64, 0x74, 0x64, 0x22, 0x3e, 0x0a, 0x0a, 
  0x3c, 0x21, 0x2d, 0x2d, 0x0a, 0x20, 0x43, 0x6f, 
  0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20, 
  0x28, 0x43, 0x29, 0x20, 0x32, 0x30, 0x31, 0x35, 
  0x20, 0x52, 0x65, 0x64, 0x20, 0x48, 0x61, 0x74, 
  0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x0a, 0x0a, 
  0x20, 0x54, 0x68, 0x69, 0x73, 0x20, 0x6c, 0x69, 
  0x62, 0x72, 0x61, 0x72, 0x79, 0x20, 0x69, 0x73, 
  0x20, 0x66, 0x72, 0x65, 0x65, 0x20, 0x73, 0x6f, 
  0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x3b, 0x20, 
  0x79, 0x6f, 0x75, 0x20, 0x63, 0x61, 0x6e, 0x20, 
  0x72, 0x65, 0x64, 0x69, 0x73, 0x74, 0x72, 0x69, 
  0x62, 0x75, 0x74, 0x65, 0x20, 0x69, 0x74, 0x20, 
  0x61, 0x6e, 0x64, 0x2f, 0x6f, 0x72, 0x0a, 0x20, 
  0x6d, 0x6f, 0x64, 0x69, 0x66, 0x79, 0x20, 0x69, 
  0x74, 0x20, 0x75, 0x6e, 0x64, 0x65, 0x72, 0x20, 
  0x74, 0x68, 0x65, 0x20, 0x74, 0x65, 0x72, 0x6d, 
  0x73, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 
  0x20, 0x47, 0x4e, 0x55, 0x20, 0x4c, 0x65, 0x73, 
  0x73, 0x65, 0x72, 0x20, 0x47, 0x65, 0x6e, 0x65, 
  0x72, 0x61, 0x6c, 0x20, 0x50, 0x75, 0x62, 0x6c, 
  0x69, 0x63, 0x0a, 0x20, 0x4c, 0x69, 0x63, 0x65, 
  0x6e, 0x73, 0x65, 0x20, 0x61, 0x73, 0x20, 0x70, 
  0x75, 0x62, 0x6c, 0x69, 0x73, 0x68, 0x65, 0x64, 
  0x20, 0x62, 0x79, 0x20, 0x74, 0x68, 0x65, 0x20, 
  0x46, 0x72, 0x65, 0x65, 0x20, 0x53, 0x6f, 0x66, 
  0x74, 0x77, 0x61, 0x72, 0x65, 0x20, 0x46, 0x6f, 
  0x75, 0x6e, 0x64, 0x61, 0x74, 0x69, 0x6f, 0x6e, 
  0x3b, 0x20, 0x65, 0x69, 0x74, 0x68, 0x65, 0x72, 
  0x0a, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 
  0x6e, 0x20, 0x32, 0x20, 0x6f, 0x66, 0x20, 0x74, 
  0x68, 0x65, 0x20, 0x4c, 0x69, 0x63, 0x65, 0x6e, 
  0x73, 0x65, 0x2c, 0x20, 0x6f, 0x72, 0x20, 0x28, 
  0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 
  0x6f, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x29, 0x20, 
  0x61, 0x6e, 0x79, 0x20, 0x6c, 0x61, 0x74, 0x65, 
  0x72, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 
  0x6e, 0x2e, 0x0a, 0x0a, 0x20, 0x54, 0x68, 0x69, 
  0x73, 0x20, 0x6c, 0x69, 0x62, 0x72, 0x61, 0x72, 
  0x79, 0x20, 0x69, 0x73, 0x20, 0x64, 0x69, 0x73, 
  0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x64, 
  0x20, 0x69, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 
  0x68, 0x6f, 0x70, 0x65, 0x20, 0x74, 0x68, 0x61, 
  0x74, 0x20, 0x69, 0x74, 0x20, 0x77, 0x69, 0x6c, 
  0x6c, 0x20, 0x62, 0x65, 0x20, 0x75, 0x73, 0x65, 
  0x66, 0x75, 0x6c, 0x2c, 0x0a, 0x20, 0x62, 0x75, 
  0x74, 0x20, 0x57, 0x49, 0x54, 0x48, 0x4f, 0x55, 
  0x54, 0x20, 0x41, 0x4e, 0x59, 0x20, 0x57, 0x41, 
  0x52, 0x52, 0x41, 0x4e, 0x54, 0x59, 0x3b, 0x20, 
  0x77, 0x69, 0x74, 0x68, 0x6f, 0x75, 0x74, 0x20, 
  0x65, 0x76, 0x65, 0x6e, 0x20, 0x74, 0x68, 0x65, 
  0x20, 0x69, 0x6d, 0x70, 0x6c, 0x69, 0x65, 0x64, 
  0x20, 0x77, 0x61, 0x72, 0x72, 0x61, 0x6e, 0x74, 
  0x79, 0x20, 0x6f, 0x66, 0x0a, 0x20, 0x4d, 0x45, 
  0x52, 0x43, 0x48, 0x41, 0x4e, 0x54, 0x41, 0x42, 
  0x49, 0x4c, 0x49, 0x54, 0x59, 0x20, 0x6f, 0x72, 
  0x20, 0x46, 0x49, 0x54, 0x4e, 0x45, 0x53, 0x53, 
  0x20, 0x46, 0x4f, 0x52, 0x20, 0x41, 0x20, 0x50, 
  0x41, 0x52, 0x54, 0x49, 0x43, 0x55, 0x4c, 0x41, 
  0x52, 0x20, 0x50, 0x55, 0x52, 0x50, 0x4f, 0x53, 
  0x45, 0x2e, 0x20, 0x20, 0x53, 0x65, 0x65, 0x20, 
  0x74, 0x68, 0x65, 0x20, 0x47, 0x4e, 0x55, 0x0a, 
  0x20, 0x4c, 0x65, 0x73, 0x73, 0x65, 0x72, 0x20, 
  0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x6c, 0x20, 
  0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20, 0x4c, 
  0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x20, 0x66, 
  0x6f, 0x72, 0x20, 0x6d, 0x6f, 0x72, 0x65, 0x20, 
  0x64, 0x65, 0x74, 0x61, 0x69, 0x6c, 0x73, 0x2e, 
  0x0a, 0x0a, 0x20, 0x59, 0x6f, 0x75, 0x20, 0x73, 
  0x68, 0x6f, 0x75, 0x6c, 0x64, 0x20, 0x68, 0x61, 
  0x76, 0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 
  0x76, 0x65, 0x64, 0x20, 0x61, 0x20, 0x63, 0x6f, 
  0x70, 0x79, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 
  0x65, 0x20, 0x47, 0x4e, 0x55, 0x20, 0x4c, 0x65, 
  0x73, 0x73, 0x65, 0x72, 0x20, 0x47, 0x65, 0x6e, 
  0x65, 0x72, 0x61, 0x6c, 0x0a, 0x20, 0x50, 0x75, 
  0x62, 0x6c, 0x69, 0x63, 0x20, 0x4c, 0x69, 0x63, 
  0x65, 0x6e, 0x73, 0x65, 0x20, 0x61, 0x6c, 0x6f, 
  0x6e, 0x67, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 
  0x74, 0x68, 0x69, 0x73, 0x20, 0x6c, 0x69, 0x62, 
  0x72, 0x61, 0x72, 0x79, 0x3b, 0x20, 0x69, 0x66, 
  0x20, 0x6e, 0x6f, 0x74, 0x2c, 0x20, 0x77, 0x72, 
  0x69, 0x74, 0x65, 0x20, 0x74, 0x6f, 0x20, 0x74, 
  0x68, 0x65, 0x0a, 0x20, 0x46, 0x72, 0x65, 0x65, 
  0x20, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 
  0x65, 0x20, 0x46, 0x6f, 0x75, 0x6e, 0x64, 0x61, 
  0x74, 0x69, 0x6f, 0x6e, 0x2c, 0x20, 0x49, 0x6e, 
  0x63, 0x2e, 0x2c, 0x20, 0x35, 0x31, 0x20, 0x46, 
  0x72, 0x61, 0x6e, 0x6b, 0x6c, 0x69, 0x6e, 0x20, 
  0x53, 0x74, 0x72, 0x65, 0x65, 0x74, 0x2c, 0x20, 
  0x46, 0x69, 0x66, 0x74, 0x68, 0x20, 0x46, 0x6c, 
  0x6f, 0x6f, 0x72, 0x2c, 0x0a, 0x20, 0x42, 0x6f, 
  0x73, 0x74, 0x6f, 0x6e, 0x2c, 0x20, 0x4d, 0x41, 
  0x20, 0x30, 0x32, 0x31, 0x31, 0x30, 0x2d, 0x31, 
  0x33, 0x30, 0x31, 0x2c, 0x20, 0x55, 0x53, 0x41, 
  0x2e, 0x0a, 0x0a, 0x20, 0x41, 0x75, 0x74, 0x68, 
  0x6f, 0x72, 0x3a, 0x20, 0x41, 0x6c, 0x65, 0x78, 
  0x61, 0x6e, 0x64, 0x65, 0x72, 0x20, 0x4c, 0x61, 
  0x72, 0x73, 0x73, 0x6f, 0x6e, 0x20, 0x3c, 0x61, 
  0x6c, 0x65, 0x78, 0x6c, 0x40, 0x72, 0x65, 0x64, 
  0x68, 0x61, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x3e, 
  0x0a, 0x2d, 0x2d, 0x3e, 0x0a, 0x0a, 0x3c, 0x6e, 
  0x6f, 0x64, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 
  0x3d, 0x22, 0x2f, 0x22, 0x20, 0x78, 0x6d, 0x6c, 
  0x6e, 0x73, 0x3a, 0x64, 0x6f, 0x63, 0x3d, 0x22, 
  0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 
  0x77, 0x77, 0x2e, 0x66, 0x72, 0x65, 0x65, 0x64, 
  0x65, 0x73, 0x6b, 0x74, 0x6f, 0x70, 0x2e, 0x6f, 
  0x72, 0x67, 0x2f, 0x64, 0x62, 0x75, 0x73, 0x2f, 
  0x31, 0x2e, 0x30, 0x2f, 0x64, 0x6f, 0x63, 0x2e, 
  0x64, 0x74, 0x64, 0x22, 0x3e, 0x0a, 0x20, 0x20, 
  0x3c, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 
  0x63, 0x65, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 
  0x27, 0x6f, 0x72, 0x67, 0x2e, 0x66, 0x72, 0x65, 
  0x65, 0x64, 0x65, 0x73, 0x6b, 0x74, 0x6f, 0x70, 
  0x2e, 0x70, 0x6f, 0x72, 0x74, 0x61, 0x6c, 0x2e, 
  0x44, 0x6f, 0x63, 0x75, 0x6d, 0x65, 0x6e, 0x74, 
  0x73, 0x27, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 
  0x3c, 0x6d, 0x65, 0x74, 0x68, 0x6f, 0x64, 0x20, 
  0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x22, 0x47, 0x65, 
  0x74, 0x4d, 0x6f, 0x75, 0x6e, 0x74, 0x50, 0x6f, 
  0x69, 0x6e, 0x74, 0x22, 0x3e, 0x0a, 0x20, 0x20, 
  0x20, 0x20, 0x20, 0x20, 0x3c, 0x61, 0x72, 0x67, 
  0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x27, 0x61, 
  0x79, 0x27, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 
  0x27, 0x70, 0x61, 0x74, 0x68, 0x27, 0x20, 0x64, 
  0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 
  0x3d, 0x27, 0x6f, 0x75, 0x74, 0x27, 0x2f, 0x3e, 
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x2f, 0x6d, 
  0x65, 0x74, 0x68, 0x6f, 0x64, 0x3e, 0x0a, 0x20, 
  0x20, 0x20, 0x20, 0x3c, 0x6d, 0x65, 0x74, 0x68, 
  0x6f, 0x64, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 
  0x22, 0x41, 0x64, 0x64, 0x22, 0x3e, 0x0a, 0x20, 
  0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x61, 0x72, 
  0x67, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x27, 
  0x68, 0x27, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 
  0x27, 0x6f, 0x5f, 0x70, 0x61, 0x74, 0x68, 0x5f, 
  0x66, 0x64, 0x27, 0x20, 0x64, 0x69, 0x72, 0x65, 
  0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3d, 0x27, 0x69, 
  0x6e, 0x27, 0x2f, 0x3e, 0x0a, 0x20, 0x20, 0x20, 
  0x20, 0x20, 0x20, 0x3c, 0x61, 0x72, 0x67, 0x20, 
  0x74, 0x79, 0x70, 0x65, 0x3d, 0x27, 0x62, 0x27, 
  0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x27, 0x72, 
  0x65, 0x75, 0x73, 0x65, 0x5f, 0x65, 0x78, 0x69, 
  0x73, 0x74, 0x69, 0x6e, 0x67, 0x27, 0x20, 0x64, 
  0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 
  0x3d, 0x27, 0x69, 0x6e, 0x27, 0x2f, 0x3e, 0x0a, 
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x61, 
  0x72, 0x67, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 
  0x27, 0x62, 0x27, 0x20, 0x6e, 0x61, 0x6d, 0x65, 
  0x3d, 0x27, 0x70, 0x65, 0x72, 0x73, 0x69, 0x73, 
  0x74, 0x65, 0x6e, 0x74, 0x27, 0x20, 0x64, 0x69, 
  0x72, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3d, 
  0x27, 0x69, 0x6e, 0x27, 0x2f, 0x3e, 0x0a, 0x20, 
  0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x61, 0x72, 
  0x67, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x27, 
  0x73, 0x27, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 
  0x27, 0x64, 0x6f, 0x63, 0x5f, 0x69, 0x64, 0x27, 
  0x20, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 
  0x6f, 0x6e, 0x3d, 0x27, 0x6f, 0x75, 0x74, 0x27, 
  0x2f, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 
  0x2f, 0x6d, 0x65, 0x74, 0x68, 0x6f, 0x64, 0x3e, 
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x6d, 0x65, 
  0x74, 0x68, 0x6f, 0x64, 0x20, 0x6e, 0x61, 0x6d, 
  0x65, 0x3d, 0x22, 0x41, 0x64, 0x64, 0x4e, 0x61, 
  0x6d, 0x65, 0x64, 0x22, 0x3e, 0x0a, 0x20, 0x20, 
  0x20, 0x20, 0x20, 0x20, 0x3c, 0x61, 0x72, 0x67, 
  0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 0x27, 0x68, 
  0x27, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x27, 
  0x6f, 0x5f, 0x70, 0x61, 0x74, 0x68, 0x5f, 0x70, 
  0x61, 0x72, 0x65, 0x6e, 0x74, 0x5f, 0x66, 0x64, 
  0x27, 0x20, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 
  0x69, 0x6f, 0x6e, 0x3d, 0x27, 0x69, 0x6e, 0x27, 
  0x2f, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 
  0x20, 0x3c, 0x61, 0x72, 0x67, 0x20, 0x74, 0x79, 
  0x70, 0x65, 0x3d, 0x27, 0x61, 0x79, 0x27, 0x20, 
  0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x27, 0x66, 0x69, 
  0x6c, 0x65, 0x6e, 0x61, 0x6d, 0x65, 0x27, 0x20, 
  0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6f, 
  0x6e, 0x3d, 0x27, 0x69, 0x6e, 0x27, 0x2f, 0x3e, 
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 
  0x61, 0x72, 0x67, 0x20, 0x74, 0x79, 0x70, 0x65, 
  0x3d, 0x27, 0x62, 0x27, 0x20, 0x6e, 0x61, 0x6d, 
  0x65, 0x3d, 0x27, 0x72, 0x65, 0x75, 0x73, 0x65, 
  0x5f, 0x65, 0x78, 0x69, 0x73, 0x74, 0x69, 0x6e, 
  0x67, 0x27, 0x20, 0x64, 0x69, 0x72, 0x65, 0x63, 
  0x74, 0x69, 0x6f, 0x6e, 0x3d, 0x27, 0x69, 0x6e, 
  0x27, 0x2f, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 
  0x20, 0x20, 0x3c, 0x61, 0x72, 0x67, 0x20, 0x74, 
  0x79, 0x70, 0x65, 0x3d, 0x27, 0x62, 0x27, 0x20, 
  0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x27, 0x70, 0x65, 
  0x72, 0x73, 0x69, 0x73, 0x74, 0x65, 0x6e, 0x74, 
  0x27, 0x20, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 
  0x69, 0x6f, 0x6e, 0x3d, 0x27, 0x69, 0x6e, 0x27, 
  0x2f, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 
  0x20, 0x3c, 0x61, 0x72, 0x67, 0x20, 0x74, 0x79, 
  0x70, 0x65, 0x3d, 0x27, 0x73, 0x27, 0x20, 0x6e, 
  0x61, 0x6d, 0x65, 0x3d, 0x27, 0x64, 0x6f, 0x63, 
  0x5f, 0x69, 0x64, 0x27, 0x20, 0x64, 0x69, 0x72, 
  0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3d, 0x27, 
  0x6f, 0x75, 0x74, 0x27, 0x2f, 0x3e, 0x0a, 0x20, 
  0x20, 0x20, 0x20, 0x3c, 0x2f, 0x6d, 0x65, 0x74, 
  0x68, 0x6f, 0x64, 0x3e, 0x0a, 0x20, 0x20, 0x20, 
  0x20, 0x3c, 0x6d, 0x65, 0x74, 0x68, 0x6f, 0x64, 
  0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x22, 0x47, 
  0x72, 0x61, 0x6e, 0x74, 0x50, 0x65, 0x72, 0x6d, 
  0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x73, 0x22, 
  0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
  0x3c, 0x61, 0x72, 0x67, 0x20, 0x74, 0x79, 0x70, 
  0x65, 0x3d, 0x27, 0x73, 0x27, 0x20, 0x6e, 0x61, 
  0x6d, 0x65, 0x3d, 0x27, 0x64, 0x6f, 0x63, 0x5f, 
  0x69, 0x64, 0x27, 0x20, 0x64, 0x69, 0x72, 0x65, 
  0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3d, 0x27, 0x69, 
  0x6e, 0x27, 0x2f, 0x3e, 0x0a, 0x20, 0x20, 0x20, 
  0x20, 0x20, 0x20, 0x3c, 0x61, 0x72, 0x67, 0x20, 
  0x74, 0x79, 0x70, 0x65, 0x3d, 0x27, 0x73, 0x27, 
  0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x27, 0x61, 
  0x70, 0x70, 0x5f, 0x69, 0x64, 0x27, 0x20, 0x64, 
  0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 
  0x3d, 0x27, 0x69, 0x6e, 0x27, 0x2f, 0x3e, 0x0a, 
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x61, 
  0x72, 0x67, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 
  0x27, 0x61, 0x73, 0x27, 0x20, 0x6e, 0x61, 0x6d, 
  0x65, 0x3d, 0x27, 0x70, 0x65, 0x72, 0x6d, 0x69, 
  0x73, 0x73, 0x69, 0x6f, 0x6e, 0x73, 0x27, 0x20, 
  0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6f, 
  0x6e, 0x3d, 0x27, 0x69, 0x6e, 0x27, 0x2f, 0x3e, 
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x2f, 0x6d, 
  0x65, 0x74, 0x68, 0x6f, 0x64, 0x3e, 0x0a, 0x20, 
  0x20, 0x20, 0x20, 0x3c, 0x6d, 0x65, 0x74, 0x68, 
  0x6f, 0x64, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 
  0x22, 0x52, 0x65, 0x76, 0x6f, 0x6b, 0x65, 0x50, 
  0x65, 0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 
  0x6e, 0x73, 0x22, 0x3e, 0x0a, 0x20, 0x20, 0x20, 
  0x20, 0x20, 0x20, 0x3c, 0x61, 0x72, 0x67, 0x20, 
  0x74, 0x79, 0x70, 0x65, 0x3d, 0x27, 0x73, 0x27, 
  0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x27, 0x64, 
  0x6f, 0x63, 0x5f, 0x69, 0x64, 0x27, 0x20, 0x64, 
  0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 
  0x3d, 0x27, 0x69, 0x6e, 0x27, 0x2f, 0x3e, 0x0a, 
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x61, 
  0x72, 0x67, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3d, 
  0x27, 0x73, 0x27, 0x20, 0x6e, 0x61, 0x6d, 0x65, 
  0x3d, 0x27, 0x61, 0x70, 0x70, 0x5f, 0x69, 0x64, 
  0x27, 0x20, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 
  0x69, 0x6f, 0x6e, 0x3d, 0x27, 0x69, 0x6e, 0x27, 
  0x2f, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 
  0x20, 0x3c, 0x61, 0x72, 0x67, 0x20, 0x74, 0x79, 
  0x70, 0x65, 0x3d, 0x27, 0x61, 0x73, 0x27, 0x20, 
  0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x27, 0x70, 0x65, 
  0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 
  0x73, 0x27, 0x20, 0x64, 0x69, 0x72, 0x65, 0x63, 
  0x74, 0x69, 0x6f, 0x6e, 0x3d, 0x27, 0x69, 0x6e, 
  0x27, 0x2f, 0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 
  0x3c, 0x2f, 0x6d, 0x65, 0x74, 0x68, 0x6f, 0x64, 
  0x3e, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x6d, 
  0x65, 0x74, 0x68, 0x6f, 0x64, 0x20, 0x6e, 0x61, 
  0x6d, 0x65, 0x3d, 0x22, 0x44, 0x65, 0x6c, 0x65, 
  0x74, 0x65, 0x22, 0x3e, 0x0a, 0x20, 0x20, 0x20, 
  0x20, 0x20, 0x20, 0x3c, 0x61, 0x72, 0x67, 0x20, 
  0x74, 0x79, 0x70, 0x65, 0x3d, 0x27, 0x73, 0x27, 
  0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x27, 0x64, 
  0x6f, 0x63, 0x5f, 0x69, 0x64, 0x27, 0x20, 0x64, 
  0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 
  0x3d, 0x27, 0x69, 0x6e, 0x27, 0x2f, 0x3e, 0x0a, 
  0x20, 0x20, 0x20, 0x20, 0x3c, 0x2f, 0x6d, 0x65, 
  0x74, 0x68, 0x6f, 0x64, 0x3e, 0x0a, 0x20, 0x20, 
  0x3c, 0x2f, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x66, 
  0x61, 0x63, 0x65, 0x3e, 0x0a, 0x3c, 0x2f, 0x6e, 
  0x6f, 0x64, 0x65, 0x3e, 0x0a, 0x00, 0x00, 0x28, 
  0x75, 0x75, 0x61, 0x79, 0x29
} };

static GStaticResource static_resource = { _xdg_document_resource_data.data, sizeof (_xdg_document_resource_data.data), NULL, NULL, NULL };
extern GResource *_xdg_document_get_resource (void);
GResource *_xdg_document_get_resource (void)
{
  return g_static_resource_get_resource (&static_resource);
}
/*
  If G_HAS_CONSTRUCTORS is true then the compiler support *both* constructors and
  destructors, in a sane way, including e.g. on library unload. If not you're on
  your own.

  Some compilers need #pragma to handle this, which does not work with macros,
  so the way you need to use this is (for constructors):

  #ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
  #pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(my_constructor)
  #endif
  G_DEFINE_CONSTRUCTOR(my_constructor)
  static void my_constructor(void) {
   ...
  }

*/

#ifndef __GTK_DOC_IGNORE__

#if  __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) static void __attribute__((constructor)) _func (void);
#define G_DEFINE_DESTRUCTOR(_func) static void __attribute__((destructor)) _func (void);

#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
/* Visual studio 2008 and later has _Pragma */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _wrapper(void) { _func(); return 0; } \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _wrapper;

#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _constructor(void) { atexit (_func); return 0; } \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _constructor;

#elif defined (_MSC_VER)

#define G_HAS_CONSTRUCTORS 1

/* Pre Visual studio 2008 must use #pragma section */
#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  section(".CRT$XCU",read)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _wrapper(void) { _func(); return 0; } \
  __declspec(allocate(".CRT$XCU")) static int (*p)(void) = _func ## _wrapper;

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  section(".CRT$XCU",read)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void); \
  static int _func ## _constructor(void) { atexit (_func); return 0; } \
  __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _constructor;

#elif defined(__SUNPRO_C)

/* This is not tested, but i believe it should work, based on:
 * http://opensource.apple.com/source/OpenSSL098/OpenSSL098-35/src/fips/fips_premain.c
 */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  init(_func)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void);

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  fini(_func)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void);

#else

/* constructors not supported for this compiler */

#endif

#endif /* __GTK_DOC_IGNORE__ */

#ifdef G_HAS_CONSTRUCTORS

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(resource_constructor)
#endif
G_DEFINE_CONSTRUCTOR(resource_constructor)
#ifdef G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(resource_destructor)
#endif
G_DEFINE_DESTRUCTOR(resource_destructor)

#else
#warning "Constructor not supported on this compiler, linking in resources will not work"
#endif

static void resource_constructor (void)
{
  g_static_resource_init (&static_resource);
}

static void resource_destructor (void)
{
  g_static_resource_fini (&static_resource);
}

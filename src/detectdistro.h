#ifndef FILE__DETECTDISTRO_H
#define FILE__DETECTDISTRO_H

#include <QString>

typedef enum{ DISTRO_DEBIAN, DISTRO_UBUNTU, DISTRO_UNKNOWN} DistroType;
void detectDistro(DistroType *type, QString *distroDesc);


#endif // FILE__DETECTDISTRO_H

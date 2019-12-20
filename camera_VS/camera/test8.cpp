#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;


img = cv2.imread('original.jpg')
gray = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
cv2.imwrite('gray.jpg', gray)
## BMP to mesh unity renderer
Small project that convernts BMP map to unity meshes and optimes the meshes along the way, old a bit clumsy version of it written in C

Program lets you draw provinces for strategy game in BMP format, then automatically converts it to objects in unity, so you don't have to create and shape them manualy

Currently unsuported, but still does the trick

# showcase video 
[![Watch the video](https://img.youtube.com/vi/vWkYJ_ajEzE/0.jpg)](https://www.youtube.com/watch?v=vWkYJ_ajEzE)

# docs 
Documentation for version 2.0


Input:
BMP image
!Black pixels are not allowed in image

Spec:
program will convert said BMP image into output data with following steps:
1.Find nonprocessed pixel (non black) and put the adres of it into free thread
2.Asign all adjacent pixles of same color into single province struct
3.Calculate variables for province, make render data (each pixel being square, 2 triangles with 4 verticies)
4.Optimize render data to reduce number of verticies and triangles by ~80%-95%, making data into big rectangles made out of 2 triangles
5.Make all pixels associated with province black
6.Free thread
<-Repeat untill all pixels are black
7.Save data into result.bin file with Output format


Output:
In Binary

```
int amountOfProvinces
[amountOfProvinces]*{
	int width of province
	int height of province
	int middleX of province 
	int middleY of province
	int amountOfVerticies
	int amountOfTrianglesPoints
	[amountOfVerticies]*{
		int x, int y
	}
	[amountOfTrianglesPoints]*{
		 int trianglePoint
	}
}
```

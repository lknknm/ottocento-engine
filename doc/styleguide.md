## Ottocento Engine Style Guide
Since this is a study project, I am very flexible about Code Style improvements and suggestions. Here are the basic rules to follow in order to style the code and make it readable.


### For the C++ library:
- Functions/Methods inside classes should be named in camelCase.

Example:
```cpp
void nameYourFunction(char* argument_name) {}
```
- Class names start with `Ott`, like `OttCamera`, `OttApplication`, etc.
- "Disposable" variables such `i, j` inside `for loops` always in lowercase.
- Indentation (Tab) with 4 spaces.
- Allman indentation style convention.
- Do not use `using namespace std` or `using namespace glm`. Namespaces and classes should be explicitly written.
- Scalars do not need to be necessarily assigned to variables. They can be just used directly in a formula to save up memory read. Example:
```cpp
EyePosition      -= rightVector * delta.x * 0.3f; // Smoothing scalar is not declared as a variable
CenterPosition   -= rightVector * delta.x * 0.3f; // Smoothing scalar is not declared as a variable
```
- One-line `if/for/foreach` statements can be left without brackets following the example:
```cpp
if (closestSphere < 0)
    return miss(ray);
```
- Functions/Methods must have a dashed-line separator before them, followed by a simple description if possible:
```cpp
// ----------------------------------------------------------------------------
// This function uses the same formula principles as the viewportInputHandle function
// for the mouse movement. The degrees for movement are calculated by multiplying:
// SIDEWAYS: angle * rotationSpeed * smoothness scalar.
// UP/DOWN:  angle * rotationSpeed * smoothness scalar / distance to the center of the camera.
// Scalars are defined arbitrarily and not assigned in order to save up some memory reads.
void OttCamera::rotateFixedAmount(rotateDirection direction)
{
    // Function
}
```
- Comments inside the methods are also allowed to better clarify the actual intent of some steps.
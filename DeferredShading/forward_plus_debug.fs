#version 330 compatibility

const int TILE_SIZE = 16;

varying vec2 texCoord;
uniform isamplerBuffer scanSum;
uniform int maxTileLights;
uniform vec2 screenSize;

void main(){
    float nCol = ceil(screenSize.x/TILE_SIZE);
    float xTile = floor(texCoord.x*screenSize.x/TILE_SIZE);
    float yTile = floor(texCoord.y*screenSize.y/TILE_SIZE);
    int tile = int(xTile+nCol*yTile);
    float c;
    if (tile == 0) c = float(texelFetch(scanSum,tile))/float(maxTileLights);
    else c = (float(texelFetch(scanSum,tile)-texelFetch(scanSum,tile-1)))/float(maxTileLights);
    gl_FragColor = vec4(c,c,c,0.0);
}
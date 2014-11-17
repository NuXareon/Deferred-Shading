#version 330 compatibility

const int TILE_SIZE = 32;

varying vec2 texCoord;
uniform samplerBuffer scanSum;
uniform int maxTileLights;
uniform vec2 screenSize;

//unifrom samplerBuffer lightList;
//uniform samplerBuffer lightsTexBuffer;
//uniform int nTiles;

//glFragCoor.xy -> tile
//tile*w/16;tile*h/16;
//lightList[scanSum[tile-1]...scanSum[tile]]
//for int i = 0; i < nTiles; ++i
//    for j = 0; j < scanSum[i]; ++j

void main(){
    float nCol = ceil(screenSize.x/TILE_SIZE);
    float nRow = ceil(screenSize.y/TILE_SIZE);
    float nTiles = nRow*nCol;
    float xTile = floor(texCoord.x*screenSize.x/TILE_SIZE);
    float yTile = floor(texCoord.y*screenSize.y/TILE_SIZE);
    int tile = int(xTile+nCol*yTile);
    float c;
    if (tile == 0) c = texelFetch(scanSum,tile).x/maxTileLights;
    else c = (texelFetch(scanSum,tile).x-texelFetch(scanSum,tile-1))/maxTileLights;
    //float c = tile/nTiles;
    gl_FragColor = vec4(c,c,c,1.0);
}
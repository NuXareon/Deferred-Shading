#version 330 compatibility

const int TILE_SIZE = 16;

varying vec2 texCoord;
uniform samplerBuffer scanSum;
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
    int nTilesLine = screenSize.x/TILE_SIZE;
    int xTile = texCoord.x*screenSize.x/TILE_SIZE;
    int yTile = texCoord.y*screenSize.y/TILE_SIZE;
    int tile = xTile+nTilesLine*yTile;
    float c = texelFetch(scanSum,tile).x;
    gl_FragColor = vec4(c,c,c,1.0);
}
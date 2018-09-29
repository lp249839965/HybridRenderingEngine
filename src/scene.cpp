// ===============================
// AUTHOR       : Angel Ortiz (angelo12 AT vt DOT edu)
// CREATE DATE  : 2018-09-12
// ===============================

//Headers
#include "scene.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>


Scene::Scene(const std::string &sceneName){
    sceneID = sceneName;
    if( !checkFileValidity(folderPath + sceneName + fileExtension) ){
        //If you do not find the scene file set the quit flag to true 
        printf("Cannot find scene descriptor file for %s \n", sceneID.c_str());
        missingScene = true; 
    }
    else{
        //Load all cameras, models and lights and return false if it fails
        missingScene = !loadContent();
    }
}

Scene::~Scene(){
    //Making sure you don't attempt to delete models that don't exist
    if (!missingScene){
        for(Model *models : modelsInScene){
            delete models;
        }
        delete mainSkyBox;
        // delete [] lights;
    }
}

//Update Order is critical for correct culling
void Scene::update(unsigned int deltaT){
    visibleModels.clear();
    mainCamera.update(deltaT);
    // for(int i=0; i < lightCount; ++i){
    //     lights[i].update(deltaT);
    // }
    for(Model *model : modelsInScene){
        model->update(deltaT);
    }
    frustrumCulling();
}
//-----------------------------GETTERS----------------------------------------------
std::vector<Model*>* Scene::getVisiblemodels(){
    return &visibleModels;
}
Camera* Scene::getCurrentCamera(){
    return &mainCamera;
}
// BaseLight * Scene::getCurrentLights(){
//     return lights;
// }
int Scene::getLightCount(){
    return lightCount;
}
//----------------------------------------------------------------

bool Scene::checkIfEmpty(){
    return missingScene;
}

Skybox *Scene::getCurrentSkybox(){
    return mainSkyBox;
}

//-----------------------------SCENE LOADING-----------------------------------

//Config file parsing, gets all the important 
bool Scene::loadContent(){
    std::string sceneConfigFilePath = folderPath + sceneID + fileExtension;
    std::ifstream file(sceneConfigFilePath.c_str());

    //Begin config file parsing
    if( !file.good() ){
        //Check config file exists
        printf("Error! Config: %s does not exist.\n", sceneConfigFilePath.c_str());
        return false;
    }
    else{
        //Parsing into Json file readable format
        json configJson;
        file >> configJson;
        //Checking that config file belongs to current scene and is properly formatted
        if(configJson["sceneID"] != sceneID){
            printf("Error! Config file: %s does not belong to current scene, check configuration.\n",sceneConfigFilePath.c_str());
            return false;
        }
        //now we can parse the rest of the file "safely"
        else{
            printf("Loading models...\n");
            loadSceneModels(configJson);

            printf("Generating skybox...\n");
            loadSkyBox(configJson);

            //lastly we check if the scene is empty and return
            return !modelsInScene.empty();
        }
    }
}

void Scene::loadSkyBox(const json &sceneConfigJson){
    std::string skyBoxName = sceneConfigJson["skybox"];
    std::string skyBoxFolderPath = "../assets/skyboxes/";
    skyBoxFolderPath += skyBoxName;

    mainSkyBox = new Skybox(skyBoxFolderPath);
    // mainSkyBox->loadCubeMap(skyBoxFolderPath);
}

void Scene::loadSceneModels(const json &sceneConfigJson ){
    //model setup
    std::string modelMesh, modelMaterial;
    TransformParameters initParameters;
    unsigned int modelCount = (unsigned int)sceneConfigJson["models"].size();
    for (unsigned int i = 0; i < modelCount; ++i)
    {
        //get model mesh and material info
        json currentModel = sceneConfigJson["models"][i];
        modelMesh = currentModel["mesh"];
        modelMaterial = currentModel["material"];

        //position
        json position = currentModel["position"];
        initParameters.translation = glm::vec3((float)position[0], (float)position[1], (float)position[2]);

        //rotation
        json rotation = currentModel["rotation"];
        initParameters.rotation = glm::vec3(glm::radians((float)rotation[0]),
                                            glm::radians((float)rotation[1]),
                                            glm::radians((float)rotation[2]));
        //scaling
        json scaling = currentModel["scaling"];
        initParameters.scaling = glm::vec3((float)scaling[0], (float)scaling[1], (float)scaling[2]);

        //attempts to load model with the initparameters it has read
        modelMesh = "../assets/models/" + sceneID + "/" + modelMesh;
        if (!checkFileValidity(modelMesh))
        {
            //If the mesh deos not exist it's very likely nothing else does, quit early
            printf("Error! Mesh: %s does not exist.\n", modelMesh.c_str());
        }
        else
        {
            modelMaterial = "../assets/models/" + modelMaterial;
            modelsInScene.push_back(new Model(modelMesh, modelMaterial, initParameters));
        }
    }
}

bool Scene::checkFileValidity(const std::string &filePath){
    struct stat info;
    //file is blocking access
    if( stat( filePath.c_str(), &info ) != 0 ){
        printf( "Cannot access %s\n", filePath.c_str() );
         return false;
    }
    else if( info.st_mode & S_IFMT ){
        //file is accessible
        printf( "%s is a valid file\n", filePath.c_str() );
        return true;
    }
    else{
        //File does not exist
        printf("Error! File: %s does not exist.\n", filePath.c_str());
        return false;
    }
}


//-------------------------------------------------------------
//TODO TODO TODO TODO TODO TODO TODO
void Scene::frustrumCulling(){
    for(Model *model : modelsInScene){
        visibleModels.push_back(model);
        // bool visible = mainCamera.checkVisibility(model->getBounds());
        // if (visible) {
        // }
    }
}













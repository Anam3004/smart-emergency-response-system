
pipeline {
    agent any

    environment {
        AWS_REGION = 'ap-south-1'
        ECR_REGISTRY = '322167800185.dkr.ecr.ap-south-1.amazonaws.com'
        ECR_REPOSITORY = 'image-regi'
        IMAGE_TAG = "${BUILD_NUMBER}"
    }

    stages {
        stage('Checkout GitHub') {
            steps {
                checkout scm
            }
        }

        stage('Build Docker Image') {
            steps {
                sh '''
                    docker build -f backend/Dockerfile \
                        -t $ECR_REGISTRY/$ECR_REPOSITORY:$IMAGE_TAG .

                    docker tag \
                        $ECR_REGISTRY/$ECR_REPOSITORY:$IMAGE_TAG \
                        $ECR_REGISTRY/$ECR_REPOSITORY:latest
                '''
            }
        }

        stage('Login to Amazon ECR') {
            steps {
                sh '''
                    aws ecr get-login-password --region $AWS_REGION |
                    docker login --username AWS --password-stdin $ECR_REGISTRY
                '''
            }
        }

        stage('Push Image to ECR') {
            steps {
                sh '''
                    docker push $ECR_REGISTRY/$ECR_REPOSITORY:$IMAGE_TAG
                    docker push $ECR_REGISTRY/$ECR_REPOSITORY:latest
                '''
            }
        }

        stage('Push to Docker Hub') {
            steps {
                withCredentials([
                    usernamePassword(
                        credentialsId: 'dockerhub-creds',
                        usernameVariable: 'DOCKERHUB_USER',
                        passwordVariable: 'DOCKERHUB_TOKEN'
                    )
                ]) {
                    sh '''
                        set -e

                        echo "$DOCKERHUB_TOKEN" | docker login \
                            --username "$DOCKERHUB_USER" \
                            --password-stdin

                        docker tag \
                            "$ECR_REGISTRY/$ECR_REPOSITORY:$IMAGE_TAG" \
                            "$DOCKERHUB_USER/sers:$IMAGE_TAG"

                        docker tag \
                            "$ECR_REGISTRY/$ECR_REPOSITORY:$IMAGE_TAG" \
                            "$DOCKERHUB_USER/sers:latest"

                        docker push "$DOCKERHUB_USER/sers:$IMAGE_TAG"
                        docker push "$DOCKERHUB_USER/sers:latest"

                        docker logout
                    '''
                }
            }
        }
    }

    post {
        always {
            sh 'docker logout "$ECR_REGISTRY" || true'
        }
        success {
            echo 'SERS image built and pushed to Amazon ECR and Docker Hub.'
        }
        failure {
            echo 'Pipeline failed. Check Console Output.'
        }
    }
}

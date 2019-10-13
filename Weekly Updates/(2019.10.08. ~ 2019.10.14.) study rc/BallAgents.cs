using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using MLAgents;

public class BallAgents : Agent
{
    private Rigidbody ballRigidbody; // 볼의 리지드바디
    public Transform pivotTransform; // 위치의 기준점
    public Transform target; // 아이템 목표

    public float moveForce = 10f; // 이동 힘


    private bool targetEaten = false; // 목표를 먹었는지
    private bool dead = false; // 사망 상태

    void Awake()
    {
        ballRigidbody = GetComponent<Rigidbody>();
    }
    void ResetTarget()
    {
        targetEaten = false;
        Vector3 randomPos = new Vector3(Random.Range(-5f, 5f), 0.5f, Random.Range(-5f, 5f));
        target.position = randomPos + pivotTransform.position;
    }

    /*
     * AgentReset(): Agent 오브젝트가 처음 활성화되거나
     * 다음 에피소드로 넘어가면서 학습환경이 초기화되거나
     * Agent가 목표를 완수/실패 하여 처음부터 다시 실행할 때 호출된다.
     */
    public override void AgentReset()
    {
        Vector3 randomPos = new Vector3(Random.Range(-5f, 5f), 0.5f, Random.Range(-5f, 5f));
        transform.position = randomPos + pivotTransform.position;

        dead = false;
        ballRigidbody.velocity = Vector3.zero; // 가속도 운동에 의한 velocity가 남아있기 때문에 이를 초기화 해준다.

        ResetTarget();
    }

    /* 
     * CollectObservations(): Agent가 주변을 관측하려 할 때마다 자동으로 호출된다.
     * CollectObservations의 workflow는 다음과 같다.
     * 1. Agent 주변 사항을 측정해서 벡터 공간이라는 곳에 삽입한다.
     * 2. Agent는 기록된 벡터 공간의 정보를 통해서 어떤 판단을 할 지 결정하게 된다.
     * 3. CollectObservations은 말그대로 Agent의 감각기관을 구성하는 것과 같다.
     * ※ 기본적으로 벡터 공간에 기록되는 값들은 정규화된 수치(-1f ~ +1f)로 기록하는 것이 퍼포먼스가 좀 더 좋다.
     */
    public override void CollectObservations()
    {
        // 보상을 유도하는 요소 - 목표와의 거리와 방향 (-5f ~ 5f)
        Vector3 distanceToTarget = target.position - transform.position;
        AddVectorObs(Mathf.Clamp(distanceToTarget.x / 5f, -1f, 1f));
        AddVectorObs(Mathf.Clamp(distanceToTarget.z / 5f, -1f, 1f));

        // 벌점을 유도하는 요소 - 발판 상에서의 위치 (= 발판 위에서 떨어지면 벌점) (-5f ~ 5f)
        Vector3 relativePos = transform.position - pivotTransform.position;
        AddVectorObs(Mathf.Clamp(relativePos.x / 5f, -1f, 1f));
        AddVectorObs(Mathf.Clamp(relativePos.z / 5f, -1f, 1f));

        // 보상과 벌점 둘 다 유도하게 되는 요소 - 오브젝트의 속도 (-10f ~ 10f)
        AddVectorObs(Mathf.Clamp(ballRigidbody.velocity.x / 10f, -1f, 1f));
        AddVectorObs(Mathf.Clamp(ballRigidbody.velocity.z / 10f, -1f, 1f));
    }

    /*
     * AgentAction(): 실제로 Agent가 선택을 통한 행동을 취하는 함수
     * vectorAction이라는 입력통로를 통해 brain이 Agent의 액션 요소를 넘겨준다
     * AgentAction()은 vectorAction으로 넘어온 값을 통해 실제 Agent들의 Action을 정의해주는 함수이다.
     * AgentAction()은 Agent들이 선택에 따라서 어떤 행동을 취할지 뿐만 아니라,
     * 직전 행동이나 지금 행동의 결과에 의해서 Agent가 어떤 보상/처벌을 받을지에 대해서도 정의해줘야 한다.
     * AgentAction()은 Agent가 행동을 하겠다라는 타이밍에 호출이 되는데 기본값으로는 Unity의 FixedTime(0.02초씩)가 동일하게 설정되어 있다.
     * 즉 선택 주기는 기본값으로 0.02초이다. (단, 이는 옵션사항이기에 조절할 수 있다.)
     */
    public override void AgentAction(float[] vectorAction, string textAction)
    {
        // 패널티 - 아무런 액션을 취하지 않을 때
        // Agent가 가만히 있어도 벌점을 받기 때문에 성급하게? Action을 취하게 만들 수 있다.
        // 단, 패널티가 너무 크면 Agent가 처벌을 피하기 위해서 일부러 죽을 수도 있다.
        AddReward(-0.001f);

        float horizontalInput = vectorAction[0];
        float verticalInput = vectorAction[1];

        ballRigidbody.AddForce(horizontalInput * moveForce, 0f, verticalInput * moveForce);

        // 보상 조건 - 타겟을 먹었을 때
        if(targetEaten)
        {
            AddReward(1.0f);
            ResetTarget();
        }
        // 벌점 조건 - Agent가 죽었을 때
        else if (dead)
        {
            AddReward(-1.0f);
            Done(); // Agent가 지금까지의 정보를 Tensorflow로 보내고, Agent의 Reset 옵션이 체크되어 있으면 Reset을 하게 한다.
        }

        // Agent의 이름과 현재까지 보상이 Agent의 머리 위에 표시
        Monitor.Log(name, GetCumulativeReward(), transform);
    }

    /* Dead Zone과 Target Item의 box collider를 통해 충돌 감지를 하게 되고
     * 이는 곧 Agent의 벌점 조건과 보상 조건에 부합되기 때문에,
     * Agent와 box collider의 트리거를 활용하기 위해 OnTrigerEnter함수를 정의한다.
     */
    void OnTriggerEnter(Collider other)
    {
        if(other.CompareTag("dead"))
        {
            dead = true;
        }
        else if(other.CompareTag("goal"))
        {
            targetEaten = true;
        }
    }
}
